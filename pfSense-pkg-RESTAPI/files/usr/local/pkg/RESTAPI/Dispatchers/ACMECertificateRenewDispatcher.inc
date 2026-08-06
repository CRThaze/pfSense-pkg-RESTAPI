<?php

namespace RESTAPI\Dispatchers;

use RESTAPI\Core\Command;
use RESTAPI\Core\Dispatcher;
use RESTAPI\Responses\ServerError;

/**
 * Defines a Dispatcher for renewing/signing an ACME certificate.
 */
class ACMECertificateRenewDispatcher extends Dispatcher {
    public array $required_packages = ['pfSense-pkg-acme'];
    public array $package_includes = [
        'acme/acme.inc',
        'acme/acme_utils.inc',
        'acme/acme_htmllist.inc',
        'acme/pkg_acme_tabs.inc',
    ];

    /**
     * Initiates the process of renewing an ACME certificate.
     */
    protected function _process(mixed ...$arguments): void {
        # Throw an error if a certificate argument was not provided
        if (!isset($arguments['certificate'])) {
            throw new ServerError(
                message: 'Failed to spawn new ACME certificate renew process: No certificate name was provided.',
                response_id: 'ACME_CERTIFICATE_RENEW_DISPATCHER_NO_CERTIFICATE_PROVIDED',
            );
        }

        # Replace the renewal result file
        $this->set_renewal_result(
            cert: $arguments['certificate'],
            last_updated: time(),
            status: 'pending',
            result_log: '',
        );

        # Record the current byte size of the acme issue log before renewal so we can
        # extract only the new content appended during this run.
        $log_file = "/tmp/acme/{$arguments['certificate']}/acme_issuecert.log";
        $pre_size = file_exists($log_file) ? filesize($log_file) : 0;

        # Renew the ACME certificate
        \pfsense_pkg\acme\issue_certificate(id: $arguments['certificate'], force: true, renew: true);

        # Extract only the bytes appended to the log since before the renewal started.
        # tail -c +N outputs from byte N onward (1-indexed), so +($pre_size+1) skips
        # all pre-existing content and returns only what was written during this run.
        $start_byte = $pre_size + 1;
        if (file_exists($log_file) && filesize($log_file) >= $start_byte) {
            $result = (new Command("/usr/bin/tail -c +$start_byte $log_file"))->output;
        } else {
            $result = '';
        }

        # Replace the renewal result file
        $this->set_renewal_result(
            cert: $arguments['certificate'],
            last_updated: time(),
            status: 'completed',
            result_log: $result,
        );

        # Wait a bit to ensure the renewal log is written before proceeding
        sleep(1);
    }

    /**
     * Sets the .renewal_result.json file for the given certificate to the given status and result log.
     */
    private function set_renewal_result(string $cert, int $last_updated, string $status, string $result_log): void {
        file_put_contents(
            "/tmp/.RESTAPI.acme.$cert.renewal_result.json",
            json_encode(['last_updated' => $last_updated, 'status' => $status, 'result_log' => $result_log]),
        );
    }
}
