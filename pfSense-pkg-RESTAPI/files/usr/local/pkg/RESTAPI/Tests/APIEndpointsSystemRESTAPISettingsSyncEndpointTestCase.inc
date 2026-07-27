<?php

namespace RESTAPI\Tests;

use RESTAPI\Core\TestCase;
use RESTAPI\Models\RESTAPISettings;
use RESTAPI\Models\User;

class APIEndpointsSystemRESTAPISettingsSyncEndpointTestCase extends TestCase {
    const ENDPOINT_URL = 'https://localhost/api/v2/system/restapi/settings/sync';

    /**
     * Enables HA sync and disables login protection before running tests.
     */
    public function setup(): void {
        $settings = new RESTAPISettings(ha_sync: true, login_protection: false);
        $settings->update();
    }

    /**
     * Restores HA sync and login protection to their original values after tests complete.
     */
    public function teardown(): void {
        $settings = new RESTAPISettings(ha_sync: false, login_protection: true);
        $settings->update();
    }

    /**
     * Ensures a POST request to the sync endpoint is rejected with a 406 when ha_sync is disabled.
     */
    public function test_post_rejected_when_ha_sync_disabled(): void {
        # Disable HA sync
        $settings = new RESTAPISettings(ha_sync: false);
        $settings->update();

        $json_resp = \RESTAPI\Core\Tools\http_request(
            url: self::ENDPOINT_URL,
            method: 'POST',
            data: ['sync_data' => json_encode(RESTAPISettings::get_pkg_config())],
            headers: ['Content-Type' => 'application/json'],
            username: 'admin',
            password: 'pfsense',
            validate_certs: false,
        );
        $resp = json_decode($json_resp);
        $this->assert_equals($resp->code, 406);
        $this->assert_equals($resp->response_id, 'RESTAPI_SETTINGS_SYNC_NOT_ENABLED');

        # Re-enable HA sync for subsequent tests
        $settings->ha_sync->value = true;
        $settings->update();
    }

    /**
     * Ensures a POST request with invalid (non-JSON) sync_data is rejected with a 400.
     */
    public function test_post_rejected_when_sync_data_is_invalid(): void {
        $json_resp = \RESTAPI\Core\Tools\http_request(
            url: self::ENDPOINT_URL,
            method: 'POST',
            data: ['sync_data' => 'not valid json'],
            headers: ['Content-Type' => 'application/json'],
            username: 'admin',
            password: 'pfsense',
            validate_certs: false,
        );
        $resp = json_decode($json_resp);
        $this->assert_equals($resp->code, 400);
        $this->assert_equals($resp->response_id, 'RESTAPI_SETTINGS_SYNC_SYNC_DATA_COULD_NOT_BE_UNSERIALIZED');
    }

    /**
     * Ensures a POST request with valid sync_data succeeds.
     */
    public function test_post_accepted_with_valid_sync_data(): void {
        $json_resp = \RESTAPI\Core\Tools\http_request(
            url: self::ENDPOINT_URL,
            method: 'POST',
            data: ['sync_data' => json_encode(RESTAPISettings::get_pkg_config())],
            headers: ['Content-Type' => 'application/json'],
            username: 'admin',
            password: 'pfsense',
            validate_certs: false,
        );
        $resp = json_decode($json_resp);
        $this->assert_equals($resp->code, 200);
    }

    /**
     * Ensures a POST request from a user without the page-all privilege is rejected with a 403.
     */
    public function test_post_rejected_without_page_all_privilege(): void {
        # Create a user with no privileges
        $user = new User(
            data: ['name' => 'sync_test_nopageall', 'password' => 'testpassword', 'priv' => []],
        );
        $user->create();

        $json_resp = \RESTAPI\Core\Tools\http_request(
            url: self::ENDPOINT_URL,
            method: 'POST',
            data: ['sync_data' => json_encode(RESTAPISettings::get_pkg_config())],
            headers: ['Content-Type' => 'application/json'],
            username: $user->name->value,
            password: 'testpassword',
            validate_certs: false,
        );
        $resp = json_decode($json_resp);
        $this->assert_equals($resp->code, 403);

        $user->delete();
    }

    /**
     * Ensures a POST request from a user with the page-all privilege is accepted.
     */
    public function test_post_accepted_with_page_all_privilege(): void {
        # Create a user with the page-all privilege
        $user = new User(
            data: ['name' => 'sync_test_pageall', 'password' => 'testpassword', 'priv' => ['page-all']],
        );
        $user->create();

        $json_resp = \RESTAPI\Core\Tools\http_request(
            url: self::ENDPOINT_URL,
            method: 'POST',
            data: ['sync_data' => json_encode(RESTAPISettings::get_pkg_config())],
            headers: ['Content-Type' => 'application/json'],
            username: $user->name->value,
            password: 'testpassword',
            validate_certs: false,
        );
        $resp = json_decode($json_resp);
        $this->assert_equals($resp->code, 200);

        $user->delete();
    }
}

