<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\NestedModelField;
use RESTAPI\Fields\StringField;

/**
 * Defines a Model that represents the live runtime status of a WireGuard tunnel. Data is sourced from wg_get_status()
 * and is read-only. Peer status information is embedded via a nested WireGuardTunnelStatusPeer model.
 */
class WireGuardTunnelStatus extends Model {
    public StringField $name;
    public StringField $status;
    public StringField $public_key;
    public StringField $listen_port;
    public IntegerField $mtu;
    public IntegerField $transfer_rx;
    public IntegerField $transfer_tx;
    public IntegerField $inpkts;
    public IntegerField $outpkts;
    public StringField $descr;
    public NestedModelField $peers;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Model attributes
        $this->internal_callable = 'get_wireguard_status';
        $this->many = true;
        $this->verbose_name = 'WireGuard Tunnel Status';
        $this->packages = ['pfSense-pkg-WireGuard'];
        $this->package_includes = [
            'wireguard/includes/wg_service.inc',
            'wireguard/includes/wg.inc',
            'wireguard/includes/wg_globals.inc',
            'wireguard/includes/wg_api.inc',
        ];

        # Fields
        $this->name = new StringField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Name',
            help_text: 'The WireGuard tunnel interface name (e.g. wg0).',
        );
        $this->status = new StringField(
            default: null,
            choices: ['up', 'down'],
            allow_null: true,
            read_only: true,
            verbose_name: 'Status',
            help_text: 'The current operational status of the tunnel interface.',
        );
        $this->public_key = new StringField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Public Key',
            help_text: 'The public key of this tunnel.',
        );
        $this->listen_port = new StringField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Listen Port',
            help_text: 'The UDP port this tunnel is listening on.',
        );
        $this->mtu = new IntegerField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'MTU',
            help_text: 'The MTU of the tunnel interface.',
        );
        $this->transfer_rx = new IntegerField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Transfer RX',
            help_text: 'The total number of bytes received on the tunnel interface.',
        );
        $this->transfer_tx = new IntegerField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Transfer TX',
            help_text: 'The total number of bytes transmitted on the tunnel interface.',
        );
        $this->inpkts = new IntegerField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Inbound Packets',
            help_text: 'The total number of inbound packets observed on the tunnel interface.',
        );
        $this->outpkts = new IntegerField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Outbound Packets',
            help_text: 'The total number of outbound packets observed on the tunnel interface.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            read_only: true,
            verbose_name: 'Description',
            help_text: 'The description of this tunnel as stored in the pfSense configuration.',
        );
        $this->peers = new NestedModelField(
            model_class: 'WireGuardPeerStatus',
            default: [],
            allow_empty: true,
            read_only: true,
            verbose_name: 'Peers',
            help_text: 'The live status of each peer associated with this WireGuard tunnel.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Retrieves the live WireGuard status from the `wg` CLI tool and maps it into the flat array format
     * expected by the Model framework's internal_callable mechanism.
     * @return array An indexed array of tunnel status records, each containing peer status as a sub-array.
     */
    protected function get_wireguard_status(): array {
        $tunnels = [];

        $this->check_packages();

        foreach (wg_get_status() as $device_name => $device) {
            # Build the indexed peer list from the public-key-keyed peers array
            $peers = [];

            foreach ($device['peers'] as $peer) {
                $peers[] = [
                    'tunnel_device' => $device_name,
                    'public_key' => $peer['public_key'] ?? null,
                    'preshared_key' => $peer['preshared_key'] ?? null,
                    'endpoint' => $peer['endpoint'] ?? null,
                    'allowed_ips' => $peer['allowed_ips'] ?? null,
                    'latest_handshake' => isset($peer['latest_handshake']) ? (int) $peer['latest_handshake'] : null,
                    'transfer_rx' => isset($peer['transfer_rx']) ? (int) $peer['transfer_rx'] : null,
                    'transfer_tx' => isset($peer['transfer_tx']) ? (int) $peer['transfer_tx'] : null,
                    'persistent_keepalive' => $peer['persistent_keepalive'] ?? null,
                    'descr' => $peer['config']['descr'] ?? '',
                ];
            }

            $tunnels[] = [
                'name' => $device_name,
                'status' => $device['status'] ?? null,
                'public_key' => $device['public_key'] ?? null,
                'listen_port' => $device['listen_port'] ?? null,
                'mtu' => isset($device['mtu']) ? (int) $device['mtu'] : null,
                'transfer_rx' => isset($device['transfer_rx']) ? (int) $device['transfer_rx'] : null,
                'transfer_tx' => isset($device['transfer_tx']) ? (int) $device['transfer_tx'] : null,
                'inpkts' => isset($device['inpkts']) ? (int) $device['inpkts'] : null,
                'outpkts' => isset($device['outpkts']) ? (int) $device['outpkts'] : null,
                'descr' => $device['config']['descr'] ?? '',
                'peers' => $peers,
            ];
        }

        return $tunnels;
    }
}
