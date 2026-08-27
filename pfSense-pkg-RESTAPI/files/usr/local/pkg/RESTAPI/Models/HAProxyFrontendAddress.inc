<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\HAProxyApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\PortField;
use RESTAPI\Fields\StringField;

/**
 * Defines a Model for HAProxy Frontend Addresses.
 */
class HAProxyFrontendAddress extends Model {
    public StringField $extaddr;
    public StringField $extaddr_custom;
    public PortField $extaddr_port;
    public BooleanField $extaddr_ssl;
    public StringField $exaddr_advanced;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->parent_model_class = 'HAProxyFrontend';
        $this->config_path = 'a_extaddr/item';
        $this->packages = ['pfSense-pkg-haproxy'];
        $this->package_includes = ['haproxy/haproxy.inc', 'haproxy/haproxy_utils.inc'];
        $this->many = true;
        $this->verbose_name = 'HAProxy Frontend Address';

        # Set model fields
        $this->extaddr = new StringField(
            required: true,
            choices_callable: 'get_extaddr_choices',
            verbose_name: 'External Address',
            help_text: 'The external address to use. In addition to the `custom`, `any_ipv4`, ' .
                '`any_ipv6`, `localhost_ipv4` and `localhost_ipv6` pseudo-addresses, each configured ' .
                "interface is selectable as `<interface>_ipv4`/`<interface>_ipv6` (e.g. `lan_ipv4`), " .
                'matching the address list offered by the HAProxy package GUI. Use `custom` with ' .
                '`extaddr_custom` to bind an arbitrary address such as a Virtual IP.',
        );
        $this->extaddr_custom = new StringField(
            required: true,
            conditions: ['extaddr' => 'custom'],
            verbose_name: 'Custom External Address',
            help_text: 'The custom IPv4 or IPv6 address to use as the external address.',
        );
        $this->extaddr_port = new PortField(
            default: null,
            allow_alias: false,
            allow_range: false,
            allow_null: true,
            verbose_name: 'External Address Port',
            help_text: 'The port to bind to for this address.',
        );
        $this->extaddr_ssl = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            verbose_name: 'External Address SSL',
            help_text: 'Enables or disables using SSL/TLS for this address.',
        );
        $this->exaddr_advanced = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Advanced External Address',
            help_text: 'The advanced configuration to apply to this address.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Builds the valid external address choices for an HAProxy frontend: the static
     * pseudo-addresses plus each configured interface's IPv4/IPv6 address (e.g.
     * `lan_ipv4`, `wan_ipv6`). This mirrors the address list the HAProxy package's
     * frontend GUI offers, so interface-bound frontends (the common case) can be
     * created through the API instead of only `custom`/`any`/`localhost`. Virtual
     * IPs continue to be bound via `custom` + `extaddr_custom`.
     * @return array The list of valid `extaddr` values.
     */
    public function get_extaddr_choices(): array {
        $choices = ['custom', 'any_ipv4', 'any_ipv6', 'localhost_ipv4', 'localhost_ipv6'];
        # Read interface identifiers straight from config (available in both the web
        # and CLI/buildschemas contexts) rather than get_configured_interface_with_descr(),
        # which is not always loaded when schemas are generated.
        foreach (array_keys(config_get_path('interfaces', [])) as $if) {
            $choices[] = $if . '_ipv4';
            $choices[] = $if . '_ipv6';
        }
        return $choices;
    }

    /**
     * Ensures haproxy is marked as dirty before applying.
     */
    public function pre_apply(): void {
        touch('/var/run/haproxy.conf.dirty');
    }

    /**
     * Applies changes to the HAProxy configuration.
     */
    public function apply(): void {
        (new HAProxyApplyDispatcher(async: $this->async))->spawn_process();
    }
}
