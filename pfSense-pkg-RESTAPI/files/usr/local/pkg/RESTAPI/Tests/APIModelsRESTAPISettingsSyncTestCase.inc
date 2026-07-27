<?php

namespace RESTAPI\Tests;

use RESTAPI\Core\TestCase;
use RESTAPI\Models\RESTAPISettings;
use RESTAPI\Models\RESTAPISettingsSync;

class APIModelsRESTAPISettingsSyncTestCase extends TestCase {
    /**
     * Enables HA sync before running tests so that requests are accepted.
     */
    public function setup(): void {
        $settings = new RESTAPISettings();
        $settings->ha_sync->value = true;
        $settings->update();
    }

    /**
     * Restores HA sync to its original disabled state after tests complete.
     */
    public function teardown(): void {
        $settings = new RESTAPISettings();
        $settings->ha_sync->value = false;
        $settings->update();
    }

    /**
     * Checks that create() is rejected with a NotAcceptableError when ha_sync is disabled.
     */
    public function test_create_rejected_when_ha_sync_disabled(): void {
        # Disable HA sync
        $settings = new RESTAPISettings();
        $settings->ha_sync->value = false;
        $settings->update();

        $this->assert_throws_response(
            response_id: 'RESTAPI_SETTINGS_SYNC_NOT_ENABLED',
            code: 406,
            callable: function () {
                $settings_sync = new RESTAPISettingsSync(
                    data: ['sync_data' => json_encode(RESTAPISettings::get_pkg_config())],
                );
                $settings_sync->create();
            },
        );

        # Re-enable HA sync for subsequent tests
        $settings->ha_sync->value = true;
        $settings->update();
    }

    /**
     * Checks that the 'validate_sync_data()' method correctly validates the 'sync_data' field.
     */
    public function test_validate_sync_data(): void {
        # Ensure a validation error is thrown if 'sync_data' is not valid JSON
        $this->assert_throws_response(
            response_id: 'RESTAPI_SETTINGS_SYNC_SYNC_DATA_COULD_NOT_BE_UNSERIALIZED',
            code: 400,
            callable: function () {
                $settings_sync = new RESTAPISettingsSync();
                $settings_sync->sync_data->value = 'not valid json';
                $settings_sync->validate();
            },
        );

        # Ensure valid JSON-encoded settings are accepted
        $this->assert_does_not_throw(
            callable: function () {
                $settings_sync = new RESTAPISettingsSync();
                $settings_sync->sync_data->value = json_encode(RESTAPISettings::get_pkg_config());
                $settings_sync->validate();
            },
        );
    }

    /**
     * Checks that the 'create()' method correctly syncs the API configuration with the JSON-encoded sync data.
     */
    public function test_create(): void {
        # Get the current API configuration
        $original_api_config = RESTAPISettings::get_pkg_config();
        $sync_api_config = $original_api_config;

        # Make some modifications to the API config and JSON-encode it
        $sync_api_config['enable'] = 'disabled';
        $sync_api_config['read_only'] = 'enabled';
        $sync_api_config['allowed_interfaces'] = 'lan';

        # Perform the sync against ourself and ensure it updates the API config
        $sync = new RESTAPISettingsSync(data: ['sync_data' => json_encode($sync_api_config)]);
        $sync->create();

        # Read the API config now and ensure it matches the config that was synced
        $new_api_config = RESTAPISettings::get_pkg_config();
        $this->assert_not_equals($original_api_config, $new_api_config);
        $this->assert_equals($sync_api_config, $new_api_config);

        # Revert the API configuration to its original values via sync
        $sync->sync_data->value = json_encode($original_api_config);
        $sync->create();

        # Read the API config now and ensure it matches the original config
        $new_api_config = RESTAPISettings::get_pkg_config();
        $this->assert_equals($original_api_config, $new_api_config);
        $this->assert_not_equals($sync_api_config, $new_api_config);
    }
}
