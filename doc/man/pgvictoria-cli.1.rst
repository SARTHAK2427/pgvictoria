==============
pgvictoria-cli
==============

--------------------------------------
Command line interface for pgvictoria
--------------------------------------

:Manual section: 1

SYNOPSIS
========

pgvictoria-cli [ -c CONFIG_FILE ] [ -u USERS_FILE ] [ -pg PG_VERSION ] [ -H HOST ] [ -P PORT ] [ -U USER ] [ -W PASSWORD ] [ -V ] [ -? ] [ COMMAND ]

DESCRIPTION
===========

pgvictoria-cli is a command-line interface utility for pgvictoria to run performance configuration comparison scans. It compares PostgreSQL server configurations against default version baselines.

OPTIONS
=======

-c, --config FILE
  Set the path to the pgvictoria.conf configuration file. Default is /etc/pgvictoria/pgvictoria.conf.

-u, --users FILE
  Set the path to the pgvictoria_users.conf configuration file. Default is /etc/pgvictoria/pgvictoria_users.conf.

-pg PG_VERSION
  Override the PostgreSQL baseline version to compare against. Valid range is 14 to 18.

-H, --host HOST
  Set the host name or IP address of the target PostgreSQL server. Default is 127.0.0.1.

-P, --port PORT
  Set the port number of the target PostgreSQL server. Default is 5432.

-U, --user USER
  Set the database user name. Default is postgres.

-W, --password PASSWORD
  Set the database password for authentication.

-V, --version
  Display version information.

-?, --help
  Display help.

COMMANDS
========

report [input_config_file] [output_html_path]
  Generate a configuration report.
  If no arguments are provided, it performs a connection-based live scan of the target PostgreSQL server.
  If one argument [input_config_file] is provided, it parses the configuration file statically and prints a plain text diff to stdout.
  If two arguments are provided, it generates a professional, monochrome, print-friendly HTML report and writes it to [output_html_path].

EXAMPLES
========

Perform a live configuration report scan:

  $ pgvictoria-cli -c pgvictoria-cli.conf report

Perform a static config file comparison and print results:

  $ pgvictoria-cli -c pgvictoria-cli.conf report /etc/postgresql/18/main/postgresql.conf

Generate an HTML configuration report for PostgreSQL 18:

  $ pgvictoria-cli -c pgvictoria-cli.conf report -pg 18 /etc/postgresql/18/main/postgresql.conf diff_report.html

REPORTING BUGS
==============

pgvictoria is under active development. Please report any bugs at
https://github.com/pgvictoria/pgvictoria/issues

COPYRIGHT
=========

pgvictoria is licensed under the 3-clause BSD License.

SEE ALSO
========

pgvictoria(1), pgvictoria-config(1)
