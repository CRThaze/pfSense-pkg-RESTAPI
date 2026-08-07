<?php

namespace RESTAPI\Tests;

use RESTAPI\Core\TestCase;
use RESTAPI\Models\HASync;

class APIModelsHASyncTestCase extends TestCase {
    /**
     * Checks that pfsync state synchronization settings can be updated without error.
     */
    public function test_update_pfsync_settings(): void {
        $this->assert_does_not_throw(
            callable: function () {
                $hasync = new HASync();
                $hasync->pfsyncenabled->value = true;
                $hasync->pfhostid->value = 'testid';
                $hasync->pfsyncpeerip->value = '192.168.1.2';
                $hasync->update();
            },
        );

        # Restore defaults
        $hasync = new HASync();
        $hasync->pfsyncenabled->value = false;
        $hasync->pfhostid->value = '';
        $hasync->pfsyncpeerip->value = '';
        $hasync->update();
    }

    /**
     * Checks that XMLRPC sync connection settings can be updated without error.
     */
    public function test_update_xmlrpc_connection_settings(): void {
        $this->assert_does_not_throw(
            callable: function () {
                $hasync = new HASync();
                $hasync->synchronizetoip->value = '10.0.0.1';
                $hasync->username->value = 'syncuser';
                $hasync->password->value = 'syncpassword';
                $hasync->adminsync->value = true;
                $hasync->update();
            },
        );

        # Restore defaults
        $hasync = new HASync();
        $hasync->synchronizetoip->value = '';
        $hasync->username->value = '';
        $hasync->password->value = '';
        $hasync->adminsync->value = false;
        $hasync->update();
    }

    /**
     * Checks that all synchronize* boolean flags can be enabled and disabled without error.
     */
    public function test_update_synchronize_flags(): void {
        $this->assert_does_not_throw(
            callable: function () {
                $hasync = new HASync(
                    data: [
                        'synchronizeusers' => true,
                        'synchronizeauthservers' => true,
                        'synchronizecerts' => true,
                        'synchronizerules' => true,
                        'synchronizeschedules' => true,
                        'synchronizealiases' => true,
                        'synchronizenat' => true,
                        'synchronizeipsec' => true,
                        'synchronizeopenvpn' => true,
                        'synchronizedhcpd' => true,
                        'synchronizedhcpdv6' => true,
                        'synchronizekea6' => true,
                        'synchronizedhcrelay' => true,
                        'synchronizedhcrelay6' => true,
                        'synchronizewol' => true,
                        'synchronizestaticroutes' => true,
                        'synchronizevirtualip' => true,
                        'synchronizetrafficshaper' => true,
                        'synchronizetrafficshaperlimiter' => true,
                        'synchronizednsforwarder' => true,
                        'synchronizecaptiveportal' => true,
                    ],
                );
                $hasync->update();
            },
        );

        # Restore all flags to disabled
        $this->assert_does_not_throw(
            callable: function () {
                $hasync = new HASync(
                    data: [
                        'synchronizeusers' => false,
                        'synchronizeauthservers' => false,
                        'synchronizecerts' => false,
                        'synchronizerules' => false,
                        'synchronizeschedules' => false,
                        'synchronizealiases' => false,
                        'synchronizenat' => false,
                        'synchronizeipsec' => false,
                        'synchronizeopenvpn' => false,
                        'synchronizedhcpd' => false,
                        'synchronizedhcpdv6' => false,
                        'synchronizekea6' => false,
                        'synchronizedhcrelay' => false,
                        'synchronizedhcrelay6' => false,
                        'synchronizewol' => false,
                        'synchronizestaticroutes' => false,
                        'synchronizevirtualip' => false,
                        'synchronizetrafficshaper' => false,
                        'synchronizetrafficshaperlimiter' => false,
                        'synchronizednsforwarder' => false,
                        'synchronizecaptiveportal' => false,
                    ],
                );
                $hasync->update();
            },
        );
    }

    /**
     * Checks that updated values are persisted correctly and can be read back.
     */
    public function test_updated_values_are_persisted(): void {
        $hasync = new HASync();
        $hasync->synchronizerules->value = true;
        $hasync->synchronizealiases->value = true;
        $hasync->username->value = 'hauser';
        $hasync->update();

        # Read the settings back and verify the values were stored
        $hasync_read = new HASync();
        $this->assert_is_true($hasync_read->synchronizerules->value);
        $this->assert_is_true($hasync_read->synchronizealiases->value);
        $this->assert_equals($hasync_read->username->value, 'hauser');

        # Restore defaults
        $hasync_read->synchronizerules->value = false;
        $hasync_read->synchronizealiases->value = false;
        $hasync_read->username->value = '';
        $hasync_read->update();
    }
}
