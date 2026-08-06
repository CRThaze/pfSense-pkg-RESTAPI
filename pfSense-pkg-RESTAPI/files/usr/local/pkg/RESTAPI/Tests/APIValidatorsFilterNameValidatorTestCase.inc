<?php

namespace RESTAPI\Tests;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\TestCase;
use RESTAPI\Validators\FilterNameValidator;

class APIValidatorsFilterNameValidatorTestCase extends TestCase {
    /**
     * Checks that a valid filter name consisting only of A-Z, a-z, 0-9, and _ passes validation.
     */
    public function test_valid_name_passes(): void {
        $this->assert_does_not_throw(callable: function () {
            $v = new FilterNameValidator();
            $v->validate('valid_Name123');
        });
    }

    /**
     * Checks that a name containing a hyphen is rejected as invalid.
     */
    public function test_rejects_hyphen(): void {
        $this->assert_throws_response(
            response_id: 'FILTER_NAME_VALIDATOR_INVALID_CHARACTERS',
            code: 400,
            callable: function () {
                $v = new FilterNameValidator();
                $v->validate('bad-name');
            },
        );
    }

    /**
     * Checks that a name containing a space is rejected as invalid.
     */
    public function test_rejects_space(): void {
        $this->assert_throws_response(
            response_id: 'FILTER_NAME_VALIDATOR_INVALID_CHARACTERS',
            code: 400,
            callable: function () {
                $v = new FilterNameValidator();
                $v->validate('bad name');
            },
        );
    }

    /**
     * Checks that a name containing a dot is rejected as invalid.
     */
    public function test_rejects_dot(): void {
        $this->assert_throws_response(
            response_id: 'FILTER_NAME_VALIDATOR_INVALID_CHARACTERS',
            code: 400,
            callable: function () {
                $v = new FilterNameValidator();
                $v->validate('bad.name');
            },
        );
    }

    /**
     * Checks that a name containing a slash is rejected as invalid.
     */
    public function test_rejects_slash(): void {
        $this->assert_throws_response(
            response_id: 'FILTER_NAME_VALIDATOR_INVALID_CHARACTERS',
            code: 400,
            callable: function () {
                $v = new FilterNameValidator();
                $v->validate('bad/name');
            },
        );
    }

    /**
     * Checks that a name containing a shell metacharacter (semicolon) is rejected as invalid.
     */
    public function test_rejects_semicolon(): void {
        $this->assert_throws_response(
            response_id: 'FILTER_NAME_VALIDATOR_INVALID_CHARACTERS',
            code: 400,
            callable: function () {
                $v = new FilterNameValidator();
                $v->validate('bad;name');
            },
        );
    }

    /**
     * Checks that a name containing an at-sign is rejected as invalid.
     */
    public function test_rejects_at_sign(): void {
        $this->assert_throws_response(
            response_id: 'FILTER_NAME_VALIDATOR_INVALID_CHARACTERS',
            code: 400,
            callable: function () {
                $v = new FilterNameValidator();
                $v->validate('bad@name');
            },
        );
    }

    /**
     * Checks that an empty string is rejected as invalid (no allowed characters present).
     */
    public function test_rejects_empty_string(): void {
        $this->assert_throws_response(
            response_id: 'FILTER_NAME_VALIDATOR_INVALID_CHARACTERS',
            code: 400,
            callable: function () {
                $v = new FilterNameValidator();
                $v->validate('');
            },
        );
    }

    /**
     * Checks that system reserved names are not allowed as filter names.
     */
    public function test_cannot_use_reserved_names(): void {
        $this->assert_throws_response(
            response_id: 'FILTER_NAME_VALIDATOR_RESERVED_NAME_USED',
            code: 400,
            callable: function () {
                $v = new FilterNameValidator();
                $v->validate('sshguard');
            },
        );
    }

    /**
     * Checks that filter names cannot begin with `pkg_`.
     */
    public function test_cannot_start_with_pkg(): void {
        $this->assert_throws_response(
            response_id: 'FILTER_NAME_VALIDATOR_CANNOT_START_WITH_PKG',
            code: 400,
            callable: function () {
                $v = new FilterNameValidator();
                $v->validate('pkg_test');
            },
        );
    }

    /**
     * Checks that a filter name that matches an existing network interface is rejected.
     */
    public function test_cannot_be_pfsense_interface_id(): void {
        $this->assert_throws_response(
            response_id: 'FILTER_NAME_VALIDATOR_NAME_IN_USE_BY_INTERFACE',
            code: 400,
            callable: function () {
                $v = new FilterNameValidator();
                $v->validate('wan');
            },
        );
    }

    /**
     * Checks that a filter name that is entirely numerical is rejected.
     */
    public function test_cannot_be_entirely_numerical(): void {
        $this->assert_throws_response(
            response_id: 'FILTER_NAME_VALIDATOR_CANNOT_BE_NUMERICAL',
            code: 400,
            callable: function () {
                $v = new FilterNameValidator();
                $v->validate('12345');
            },
        );
    }

    /**
     * Checks that the invalid-characters check fires before the reserved-name check, ensuring
     * character validation is the outermost guard.
     */
    public function test_invalid_characters_checked_before_reserved_names(): void {
        $this->assert_throws_response(
            response_id: 'FILTER_NAME_VALIDATOR_INVALID_CHARACTERS',
            code: 400,
            callable: function () {
                # 'ssh guard' (with a space) would also be a reserved name pattern, but the
                # invalid-character error must be raised first.
                $v = new FilterNameValidator();
                $v->validate('ssh guard');
            },
        );
    }
}
