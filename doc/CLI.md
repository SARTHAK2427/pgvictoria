# pgvictoria Command-Line Interface (pgvictoria-cli)

`pgvictoria-cli` is the command-line utility used to generate performance and configuration reports for PostgreSQL, either online by query execution on active servers or offline directly on configuration files.

## Usage

```bash
pgvictoria-cli [ -c CONFIG_FILE ] [ -u USERS_FILE ] [ -pg PG_VERSION ] [ COMMAND ]
```

## Options

*   **-c, --config CONFIG_FILE**
    Set the path to the main `pgvictoria.conf` configuration file (defaults to `/etc/pgvictoria/pgvictoria.conf`).

*   **-u, --users USERS_FILE**
    Set the path to the `pgvictoria_users.conf` configuration file (defaults to `/etc/pgvictoria/pgvictoria_users.conf`).

*   **-H, --host HOST**
    Set the host name or IP address of the target PostgreSQL server (defaults to `127.0.0.1`).

*   **-P, --port PORT**
    Set the port number of the target PostgreSQL server (defaults to `5432`).

*   **-U, --user USER**
    Set the database user name (defaults to `postgres`).

*   **-W, --password PASSWORD**
    Set the database password for authentication.

*   **-pg, --pg PG_VERSION**
    Override the PostgreSQL baseline version to compare against. Useful in offline file reporting modes when no version can be auto-detected. Valid values are `14` to `18`.

*   **-V, --version**
    Display version information.

*   **-?, --help**
    Display the help and usage guide.

## Commands

### report

Generates a configuration comparison report against the target version's default out-of-the-box configuration baseline.

```bash
pgvictoria-cli report [input_config_file] [output_html_path]
```

#### Online Mode (Zero Arguments)
Runs a connection-based configuration scan against the target PostgreSQL server.
```bash
pgvictoria-cli -c pgvictoria-cli.conf report
```

#### Offline File Mode (One Argument)
Runs a static scan comparing `<input_config_file>` against the detected version default, printing the diff report directly to `stdout`.
```bash
pgvictoria-cli -c pgvictoria-cli.conf report /etc/postgresql/18/main/postgresql.conf
```

#### Programmatic HTML Report (Two Arguments)
Runs a static scan comparing `<input_config_file>` against the detected/overridden version default, generating a clean, high-contrast, professional monochrome HTML report written to `<output_html_path>`.
```bash
pgvictoria-cli -c pgvictoria-cli.conf report -pg 18 /etc/postgresql/18/main/postgresql.conf diff_report.html
```
