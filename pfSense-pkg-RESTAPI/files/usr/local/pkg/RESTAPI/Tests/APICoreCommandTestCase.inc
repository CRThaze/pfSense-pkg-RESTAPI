<?php

namespace RESTAPI\Tests;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Command;
use RESTAPI\Core\TestCase;

class APICoreCommandTestCase extends TestCase {
    /**
     * Checks that the Command object successfully runs the requested command and returns the correct output and
     * result code. Escaping is disabled here because these commands rely on raw shell behaviour (e.g. a bare
     * unrecognised token and a pre-quoted echo argument).
     */
    public function test_command(): void {
        # Run a command that doesn't exist, ensure it's output and result code are expected
        $cmd = new Command('asdf', escape: false);
        $this->assert_equals($cmd->output, 'sh: asdf: not found');
        $this->assert_equals($cmd->result_code, 127);

        # Run an echo command, ensure it's output and result code are expected
        $cmd = new Command("echo 'test'", escape: false);
        $this->assert_equals($cmd->output, 'test');
        $this->assert_equals($cmd->result_code, 0);
    }

    /**
     * Checks that escape defaults to true when not explicitly specified.
     */
    public function test_escape_defaults_to_true(): void {
        $cmd = new Command('echo hello');
        $this->assert_is_true($cmd->escape);
    }

    /**
     * Checks that escape can be explicitly disabled via named argument.
     */
    public function test_escape_can_be_disabled(): void {
        $cmd = new Command('echo hello', escape: false);
        $this->assert_is_false($cmd->escape);
    }

    /**
     * Checks that a simple auto-escaped command runs successfully and returns expected output.
     */
    public function test_auto_escape_basic_execution(): void {
        $cmd = new Command('echo hello');
        $this->assert_equals($cmd->output, 'hello');
        $this->assert_equals($cmd->result_code, 0);
    }

    /**
     * Checks that a Command with escape disabled (false) allows compound shell expressions to run.
     */
    public function test_escape_false_allows_raw_shell_string(): void {
        $cmd = new Command('echo foo && echo bar', escape: false);
        $this->assert_str_contains($cmd->output, 'foo');
        $this->assert_str_contains($cmd->output, 'bar');
        $this->assert_equals($cmd->result_code, 0);
    }

    /**
     * Checks that tokenize_command splits a simple unquoted command into the expected tokens.
     */
    public function test_tokenize_command_simple(): void {
        $tokens = Command::tokenize_command('/sbin/ifconfig em0 -group mygroup');
        $this->assert_equals($tokens, ['/sbin/ifconfig', 'em0', '-group', 'mygroup']);
    }

    /**
     * Checks that tokenize_command returns an empty array for an empty string.
     */
    public function test_tokenize_command_empty_string(): void {
        $tokens = Command::tokenize_command('');
        $this->assert_equals($tokens, []);
    }

    /**
     * Checks that tokenize_command preserves a space inside single-quoted substrings as one token
     * and strips the surrounding quotes.
     */
    public function test_tokenize_command_single_quoted_token(): void {
        $tokens = Command::tokenize_command("echo 'hello world'");
        $this->assert_equals($tokens, ['echo', 'hello world']);
    }

    /**
     * Checks that tokenize_command preserves a space inside double-quoted substrings as one token
     * and strips the surrounding quotes.
     */
    public function test_tokenize_command_double_quoted_token(): void {
        $tokens = Command::tokenize_command('echo "hello world"');
        $this->assert_equals($tokens, ['echo', 'hello world']);
    }

    /**
     * Checks that tokenize_command handles a mix of quoted and unquoted tokens in one string.
     */
    public function test_tokenize_command_mixed_quoted_and_unquoted(): void {
        $tokens = Command::tokenize_command('/bin/cmd --flag "value with spaces" bare');
        $this->assert_equals($tokens, ['/bin/cmd', '--flag', 'value with spaces', 'bare']);
    }

    /**
     * Checks that escape_command wraps each unquoted token with escapeshellarg() single-quote quoting.
     */
    public function test_escape_command_wraps_tokens_in_single_quotes(): void {
        $escaped = Command::escape_command('/sbin/ifconfig em0');
        $this->assert_equals($escaped, "'/sbin/ifconfig' 'em0'");
    }

    /**
     * Checks that escape_command re-escapes a token that was already single-quoted in the input.
     */
    public function test_escape_command_re_escapes_pre_single_quoted_token(): void {
        $escaped = Command::escape_command("/sbin/ifconfig em0 -group 'mygroup'");
        $this->assert_equals($escaped, "'/sbin/ifconfig' 'em0' '-group' 'mygroup'");
    }

    /**
     * Checks that escape_command re-escapes a token that was already double-quoted in the input.
     */
    public function test_escape_command_re_escapes_pre_double_quoted_token(): void {
        $escaped = Command::escape_command('/sbin/ifconfig em0 -group "mygroup"');
        $this->assert_equals($escaped, "'/sbin/ifconfig' 'em0' '-group' 'mygroup'");
    }

    /**
     * Checks that escape_command neutralises a semicolon injection payload so it cannot act as a
     * shell command separator.
     */
    public function test_escape_command_neutralises_semicolon_injection(): void {
        $escaped = Command::escape_command('/bin/cmd arg; rm -rf /');
        $this->assert_str_does_not_contain($escaped, ';');
    }

    /**
     * Checks that escape_command neutralises $(...) command substitution syntax.
     */
    public function test_escape_command_neutralises_command_substitution(): void {
        $escaped = Command::escape_command('/bin/cmd $(evil)');
        $this->assert_str_does_not_contain($escaped, '$(');
    }

    /**
     * Checks that escape_command neutralises backtick command substitution syntax.
     */
    public function test_escape_command_neutralises_backtick_substitution(): void {
        $escaped = Command::escape_command('/bin/cmd `evil`');
        $this->assert_str_does_not_contain($escaped, '`');
    }

    /**
     * Checks that escape_command neutralises a pipe character so it cannot chain commands.
     */
    public function test_escape_command_neutralises_pipe(): void {
        $escaped = Command::escape_command('/bin/cmd arg | cat /etc/passwd');
        $this->assert_str_does_not_contain($escaped, '|');
    }

    /**
     * Checks that escape_command neutralises an ampersand so it cannot background or chain commands.
     */
    public function test_escape_command_neutralises_ampersand(): void {
        $escaped = Command::escape_command('/bin/cmd arg && evil');
        # The && must not survive as shell syntax
        $this->assert_str_does_not_contain($escaped, '&&');
    }

    /**
     * Checks that auto-escaping prevents injection via a malicious value interpolated into the
     * command string — the injected shell code must not be executed.
     */
    public function test_auto_escape_prevents_injection_via_interpolated_value(): void {
        $sentinel = '/tmp/restapi_cmd_injection_test_' . getmypid();

        # Simulate a field value that contains an injection payload
        $malicious_value = "em0; touch $sentinel";
        new Command("/sbin/ifconfig $malicious_value -group testgroup");

        $this->assert_is_false(file_exists($sentinel));
    }

    /**
     * Checks that auto-escaping correctly handles a token whose inner value contains a single quote,
     * producing a safely escaped shell argument rather than a broken quoting sequence.
     */
    public function test_auto_escape_handles_embedded_single_quote_in_value(): void {
        # escapeshellarg() should encode the apostrophe safely
        $escaped = Command::escape_command("/bin/echo it's");
        $this->assert_is_true(strlen($escaped) > 0);

        # The shell must not crash and must exit 0
        $cmd = new Command("/bin/echo it's");
        $this->assert_equals($cmd->result_code, 0);
    }

    /**
     * Checks that auto-escaping correctly handles a token whose inner value contains a double quote.
     */
    public function test_auto_escape_handles_embedded_double_quote_in_value(): void {
        $cmd = new Command('/bin/echo say "hello"');
        $this->assert_equals($cmd->result_code, 0);
    }

    /**
     * Checks that trim_whitespace still works correctly when escape is disabled.
     */
    public function test_trim_whitespace_with_escape_disabled(): void {
        $cmd = new Command('printf "foo   bar"', trim_whitespace: true, escape: false);
        $this->assert_equals($cmd->output, 'foo bar');
    }

    /**
     * Checks that a basic pipe is constructed and both commands run — output of the first feeds the second.
     */
    public function test_pipe_basic(): void {
        # echo three lines; grep filters to only lines containing 'keep'
        $cmd = new Command(
            command: 'printf "keep\nskip\nkeep2"',
            escape: false,
            pipe: 'grep keep',
            escape_pipe: false,
        );
        $this->assert_str_contains($cmd->output, 'keep');
        $this->assert_str_does_not_contain($cmd->output, 'skip');
    }

    /**
     * Checks that the pipe command receives the correct output when the primary command is auto-escaped.
     */
    public function test_pipe_with_escaped_primary_command(): void {
        $cmd = new Command(
            command: 'echo hello',
            pipe: 'grep hello',
            escape_pipe: false,
        );
        $this->assert_str_contains($cmd->output, 'hello');
        $this->assert_equals($cmd->result_code, 0);
    }

    /**
     * Checks that the pipe stage itself is auto-escaped when escape_pipe is true (default), preventing
     * injection through the pipe argument.
     */
    public function test_pipe_escape_pipe_default_true(): void {
        $sentinel = '/tmp/restapi_pipe_injection_test_' . getmypid();

        # Attempt injection through the pipe string — if escape_pipe works, touch must not run
        $cmd = new Command(
            command: 'echo hello',
            pipe: "grep hello; touch $sentinel",
            escape_pipe: true,
        );

        $this->assert_is_false(file_exists($sentinel));
    }

    /**
     * Checks that escape_pipe defaults to true when not explicitly specified.
     */
    public function test_pipe_escape_pipe_defaults_to_true(): void {
        $cmd = new Command('echo hello', pipe: 'grep hello');
        $this->assert_is_true($cmd->escape_pipe);
    }

    /**
     * Checks that escape_pipe can be set to false, allowing raw shell syntax in the pipe stage.
     */
    public function test_pipe_escape_pipe_false_allows_raw_syntax(): void {
        # grep with a quoted pattern only works when escape_pipe is false
        $cmd = new Command(
            command: 'printf "hello world\ngoodbye"',
            escape: false,
            pipe: 'grep "hello world"',
            escape_pipe: false,
        );
        $this->assert_str_contains($cmd->output, 'hello world');
        $this->assert_str_does_not_contain($cmd->output, 'goodbye');
    }

    /**
     * Checks that an empty pipe string does not alter the command — no pipe is inserted.
     */
    public function test_pipe_empty_string_does_not_pipe(): void {
        $cmd = new Command('echo hello', pipe: '');
        $this->assert_equals($cmd->output, 'hello');
        $this->assert_equals($cmd->result_code, 0);
    }

    /**
     * Checks that the $pipe property is stored correctly on the object.
     */
    public function test_pipe_property_stored(): void {
        $cmd = new Command('echo hello', pipe: 'grep hello', escape_pipe: false);
        $this->assert_equals($cmd->pipe, 'grep hello');
        $this->assert_is_false($cmd->escape_pipe);
    }
}
