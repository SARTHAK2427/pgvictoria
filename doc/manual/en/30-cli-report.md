# CLI Reporting Engine

The `pgvictoria-cli` report command parses target PostgreSQL configuration parameters and compares them against standard out-of-the-box baselines. This enables rapid detection of configurations that deviate from default values, streamlining performance audits and tuning tasks.

## Overview

The reporting engine supports:
*   **Online scan**: Querying active configurations directly from a live PostgreSQL instance.
*   **Offline scan**: Performing static difference analysis on a local `postgresql.conf` file.
*   **Version override**: Forcing audits against a specific PostgreSQL baseline version (14 through 18).
*   **HTML report**: Exporting audits to clean, professional, high-contrast monochrome HTML documents.

## Usage

### Direct Command Line Output
To view configuration differences in your terminal, run the `report` command with the path to the PostgreSQL configuration file:

```bash
pgvictoria-cli -c pgvictoria-cli.conf report /etc/postgresql/18/main/postgresql.conf
```

This prints a text-based diff showing the baseline defaults versus the values defined in the file.

### Programmatic HTML Reports
To export configuration audits to an HTML report, pass the output HTML filename as the second argument:

```bash
pgvictoria-cli -c pgvictoria-cli.conf report /etc/postgresql/18/main/postgresql.conf report.html
```

### Forcing baseline versions
If the configuration file does not declare its version in comments, or if you want to inspect how your configuration compares to a different PostgreSQL release, use the `-pg` (or `--pg`) override flag:

```bash
pgvictoria-cli -c pgvictoria-cli.conf report -pg 18 /etc/postgresql/18/main/postgresql.conf report.html
```

## Security

`pgvictoria-cli` report features comply with standard safety policies:
*   **No Root Execution**: It refuses to run if invoked by the `root` account to prevent unintentional read/write actions on sensitive system files.
*   **Offline Isolation**: The configuration file reporting mode is completely offline. It reads settings locally and compares them against compiled baseline header definitions without initiating database connections or transmitting configuration parameters.
*   **Buffer Safety**: Command-line arguments (like filenames or version inputs) are parsed and copied using safe `pgvictoria_append()` allocation and size-bounded `snprintf()` calls rather than unsafe raw copying operations.
