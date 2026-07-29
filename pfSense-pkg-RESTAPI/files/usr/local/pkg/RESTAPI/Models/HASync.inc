<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\FirewallApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\InterfaceField;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\IPAddressValidator;

/**
 * Defines a Model for managing pfSense High Availability synchronization settings.
 */
class HASync extends Model {
    public StringField $synchronizetoip;
    public StringField $pfsyncpeerip;
    public InterfaceField $pfsyncinterface;
    public StringField $pfhostid;
    public StringField $username;
    public StringField $password;
    public BooleanField $pfsyncenabled;
    public BooleanField $adminsync;
    public BooleanField $synchronizeusers;
    public BooleanField $synchronizeauthservers;
    public BooleanField $synchronizecerts;
    public BooleanField $synchronizerules;
    public BooleanField $synchronizeschedules;
    public BooleanField $synchronizealiases;
    public BooleanField $synchronizenat;
    public BooleanField $synchronizeipsec;
    public BooleanField $synchronizeopenvpn;
    public BooleanField $synchronizedhcpd;
    public BooleanField $synchronizedhcpdv6;
    public BooleanField $synchronizekea6;
    public BooleanField $synchronizewol;
    public BooleanField $synchronizestaticroutes;
    public BooleanField $synchronizevirtualip;
    public BooleanField $synchronizetrafficshaper;
    public BooleanField $synchronizetrafficshaperlimiter;
    public BooleanField $synchronizednsforwarder;
    public BooleanField $synchronizecaptiveportal;
    public BooleanField $synchronizedhcrelay;
    public BooleanField $synchronizedhcrelay6;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set Model attributes
        $this->config_path = 'hasync';
        $this->many = false;
        $this->always_apply = true;
        $this->verbose_name = 'HA Sync Settings';
        $this->verbose_name_plural = 'HA Sync Settings';

        $this->pfsyncenabled = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: null,
            help_text: 'Enable pfsync state synchronization.',
        );
        $this->pfsyncinterface = new InterfaceField(
            default: 'lo0',
            allow_localhost_interface: true,
            allow_empty: true,
            help_text: 'The interface used by pfsync state synchronization.',
        );
        $this->pfhostid = new StringField(
            default: '',
            allow_empty: true,
            maximum_length: 8,
            help_text: 'Custom pf host identifier carried in state data.',
        );
        $this->pfsyncpeerip = new StringField(
            default: '',
            allow_empty: true,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: false)],
            help_text: 'The peer IP address used by pfsync.',
        );
        $this->synchronizetoip = new StringField(
            default: '',
            allow_empty: true,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: false)],
            help_text: 'The remote pfSense host IP address used for XMLRPC configuration synchronization.',
        );
        $this->username = new StringField(
            default: '',
            allow_empty: true,
            help_text: 'The remote pfSense username used for XMLRPC synchronization.',
        );
        $this->password = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            sensitive: true,
            internal_name: 'passwordfld',
            help_text: 'The remote pfSense password used for XMLRPC synchronization.',
        );
        $this->adminsync = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: null,
            help_text: 'Synchronize admin accounts and automatically update the XMLRPC sync password.',
        );
        $this->synchronizeusers = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: null,
            help_text: 'Synchronize users and groups.',
        );
        $this->synchronizeauthservers = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: null,
            help_text: 'Synchronize authentication servers.',
        );
        $this->synchronizecerts = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: null,
            help_text: 'Synchronize certificates.',
        );
        $this->synchronizerules = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: null,
            help_text: 'Synchronize firewall rules.',
        );
        $this->synchronizeschedules = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: null,
            help_text: 'Synchronize firewall schedules.',
        );
        $this->synchronizealiases = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: null,
            help_text: 'Synchronize firewall aliases.',
        );
        $this->synchronizenat = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: null,
            help_text: 'Synchronize NAT configuration.',
        );
        $this->synchronizeipsec = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: null,
            help_text: 'Synchronize IPsec configuration.',
        );
        $this->synchronizeopenvpn = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: null,
            help_text: 'Synchronize OpenVPN configuration.',
        );
        $this->synchronizedhcpd = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: null,
            help_text: 'Synchronize DHCP server configuration.',
        );
        $this->synchronizedhcpdv6 = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: null,
            help_text: 'Synchronize DHCPv6 server configuration.',
        );
        $this->synchronizekea6 = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: null,
            help_text: 'Synchronize Kea DHCPv6 server configuration.',
        );
        $this->synchronizedhcrelay = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: null,
            help_text: 'Synchronize DHCP relay configuration.',
        );
        $this->synchronizedhcrelay6 = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: null,
            help_text: 'Synchronize DHCPv6 relay configuration.',
        );
        $this->synchronizewol = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: null,
            help_text: 'Synchronize Wake-on-LAN configuration.',
        );
        $this->synchronizestaticroutes = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: null,
            help_text: 'Synchronize static routes.',
        );
        $this->synchronizevirtualip = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: null,
            help_text: 'Synchronize virtual IP addresses.',
        );
        $this->synchronizetrafficshaper = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: null,
            help_text: 'Synchronize traffic shaper queues.',
        );
        $this->synchronizetrafficshaperlimiter = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: null,
            help_text: 'Synchronize traffic shaper limiters.',
        );
        $this->synchronizednsforwarder = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: null,
            help_text: 'Synchronize DNS Forwarder and DNS Resolver configuration.',
        );
        $this->synchronizecaptiveportal = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: null,
            help_text: 'Synchronize captive portal configuration.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Applies HA Sync configuration changes via the FirewallApplyDispatcher.
     */
    public function apply(): void {
        (new FirewallApplyDispatcher(async: $this->async))->spawn_process();
    }
}
