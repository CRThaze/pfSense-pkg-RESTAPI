<?php

namespace RESTAPI\Core;

/**
 * Defines an object that executes shell commands and keeps track of the output and result codes.
 */
class Command {
    use BaseTraits;

    /**
     * @var string $command The shell command this object should execute.
     */
    public string $command;

    /**
     * @var string $redirect An optional shell redirect to append to the end of the $command.
     */
    public string $redirect;

    /**
     * @var string $output The output of the executed shell command.
     */
    public string $output = '';

    /**
     * @var int $result_code The exit/return code of the executed shell command.
     */
    public int $result_code = -1;

    /**
     * @var bool $trim_whitespace Automatically remove excessive whitespace from the command $output.
     */
    public bool $trim_whitespace = false;

    /**
     * @var bool $escape Whether to automatically escape each token in the command string before execution.
     * Defaults to true. Set to false only when the caller has already performed its own escaping or when
     * shell features such as pipes, redirects, or glob expansion are intentionally required.
     */
    public bool $escape = true;

    /**
     * @var string $pipe An optional command string to pipe the output of $command into. When set, the shell
     * pipeline `<command> | <pipe>` is constructed. The pipe command is escaped independently according to
     * $escape_pipe. Use this instead of embedding `|` in the raw $command string, which would be neutralised
     * by auto-escaping.
     */
    public string $pipe = '';

    /**
     * @var bool $escape_pipe Whether to automatically escape the tokens of the $pipe command string. Defaults
     * to true. Set to false when the pipe command contains shell features (e.g. `grep "some pattern"`) that
     * must be passed verbatim.
     */
    public bool $escape_pipe = true;

    /**
     * Defines the Command object including the shell command to execute and optional modifiers. Note: By default,
     * the command output will redirect stderr to stdout so error message will be included in the output.
     * @param string $command The shell command to execute.
     * @param bool $trim_whitespace Remove excessive whitespace from the command output.
     * @param string $redirect An optional shell redirect to append to the end of the $command.
     * @param bool $escape Automatically escape each token in the command string before execution. Defaults to
     * true. Set to false only when the caller has already performed its own escaping or when shell features
     * such as pipes, redirects, or glob expansion are intentionally required.
     * @param string $pipe An optional command string to pipe the output of $command into. Escaped independently
     * according to $escape_pipe.
     * @param bool $escape_pipe Whether to automatically escape the tokens of the $pipe command. Defaults to true.
     * @return Command Returns this object containing the results of the executed command. Note: the object returned
     * cannot be used to initiate new commands. A new Command object should be created for any additional commands.
     */
    public function __construct(
        string $command,
        bool $trim_whitespace = false,
        string $redirect = '2>&1',
        bool $escape = true,
        string $pipe = '',
        bool $escape_pipe = true,
    ) {
        $this->command = $command;
        $this->trim_whitespace = $trim_whitespace;
        $this->redirect = $redirect;
        $this->escape = $escape;
        $this->pipe = $pipe;
        $this->escape_pipe = $escape_pipe;
        $this->run_command();
        return $this;
    }

    /**
     * Parses a raw command string into an array of argument tokens, respecting single- and double-quoted
     * substrings so that spaces inside quotes are not treated as delimiters. Surrounding quotes are stripped
     * from each token so that the value can be re-escaped uniformly by escape_command().
     * @param string $command The raw command string to tokenize.
     * @return array<string> An ordered list of unquoted token strings.
     */
    public static function tokenize_command(string $command): array {
        $tokens = [];

        # Match single-quoted strings, double-quoted strings, or unquoted non-whitespace sequences
        preg_match_all("/'[^']*'|\"[^\"]*\"|[^\s]+/", $command, $matches);

        foreach ($matches[0] as $token) {
            # Strip surrounding single quotes
            if (str_starts_with($token, "'") && str_ends_with($token, "'")) {
                $tokens[] = substr($token, 1, -1);
            }
            # Strip surrounding double quotes
            elseif (str_starts_with($token, '"') && str_ends_with($token, '"')) {
                $tokens[] = substr($token, 1, -1);
            }
            # Unquoted token — use as-is
            else {
                $tokens[] = $token;
            }
        }

        return $tokens;
    }

    /**
     * Escapes a raw command string by tokenizing it (respecting quoted substrings) and applying
     * escapeshellarg() to every token before re-joining with spaces. This prevents command injection
     * regardless of whether the original string contained pre-quoted or unquoted arguments.
     * @param string $command The raw command string to escape.
     * @return string The fully escaped command string ready for exec().
     */
    public static function escape_command(string $command): string {
        $tokens = self::tokenize_command($command);
        $escaped = array_map('escapeshellarg', $tokens);
        return implode(' ', $escaped);
    }

    /**
     * Executes the assigned $command. The $output and $result_code properties will be set after running
     * this method.
     */
    private function run_command(): void {
        $safe_command = $this->escape ? self::escape_command($this->command) : $this->command;

        # Append a pipe stage when one is specified, escaping it independently
        if ($this->pipe !== '') {
            $safe_pipe = $this->escape_pipe ? self::escape_command($this->pipe) : $this->pipe;
            $safe_command = "$safe_command | $safe_pipe";
        }

        exec(command: "$safe_command $this->redirect", output: $raw_output, result_code: $this->result_code);
        $this->output = implode(PHP_EOL, $raw_output);

        # Normalize output's whitespace if requested
        if ($this->trim_whitespace) {
            $this->output = preg_replace('/\s+/', ' ', $this->output);
        }
    }
}
