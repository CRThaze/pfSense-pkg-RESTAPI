<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;
use RESTAPI\Fields\UnixTimeField;

/**
 * Defines a Model that represents the live status of a single WireGuard peer. This Model is used as a nested object
 * within WireGuardTunnelStatus and is read-only (backed by wg_get_status() via the parent model).
 */
class WireGuardPeerStatus extends Model {
    public StringField $tunnel_device;
    public StringField $public_key;
    public StringField $preshared_key;
    public StringField $endpoint;
    public StringField $allowed_ips;
    public UnixTimeField $latest_handshake;
    public IntegerField $transfer_rx;
    public IntegerField $transfer_tx;
    public StringField $persistent_keepalive;
    public StringField $descr;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Model attributes
        $this->many = true;
        $this->internal_callable = 'get_wireguard_status';
        $this->verbose_name = 'WireGuard Tunnel Status Peer';
        $this->packages = ['pfSense-pkg-WireGuard'];
        $this->package_includes = [
            'wireguard/includes/wg_service.inc',
            'wireguard/includes/wg.inc',
            'wireguard/includes/wg_globals.inc',
            'wireguard/includes/wg_api.inc',
        ];

        # Fields
        $this->tunnel_device = new StringField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Tunnel Device',
            help_text: 'The tunnel device this peer is associated with.',
        );
        $this->public_key = new StringField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Public Key',
            help_text: 'The public key of the peer.',
        );
        $this->preshared_key = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            sensitive: true,
            verbose_name: 'Preshared Key',
            help_text: 'The preshared key negotiated with this peer, if any.',
        );
        $this->endpoint = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'Endpoint',
            help_text: 'The remote endpoint (IP:port) the peer is connected from.',
        );
        $this->allowed_ips = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            many: true,
            verbose_name: 'Allowed IPs',
            help_text: 'The comma-separated list of IP prefixes that are routed to this peer.',
        );
        $this->latest_handshake = new UnixTimeField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Latest Handshake',
            help_text: 'The Unix timestamp of the most recent handshake with this peer.',
        );
        $this->transfer_rx = new IntegerField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Transfer RX',
            help_text: 'The total number of bytes received from this peer.',
        );
        $this->transfer_tx = new IntegerField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Transfer TX',
            help_text: 'The total number of bytes sent to this peer.',
        );
        $this->persistent_keepalive = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'Persistent Keepalive',
            help_text: 'The persistent keepalive interval for this peer, or "off" if disabled.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            read_only: true,
            verbose_name: 'Description',
            help_text: 'The description of this peer as stored in the pfSense configuration.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Retrieves the live WireGuard status from the `wg` CLI tool and maps it into the flat array format
     * expected by the Model framework's internal_callable mechanism.
     * @return array An indexed array of tunnel status records, each containing peer status as a sub-array.
     */
    protected function get_wireguard_status(): array {
        $peers = [];
        $tunnels = (new WireGuardTunnelStatus())->get_internal_objects();

        foreach ($tunnels as $tunnel) {
            $peers = $peers + $tunnel['peers'];
        }

        return $peers;
    }
}
