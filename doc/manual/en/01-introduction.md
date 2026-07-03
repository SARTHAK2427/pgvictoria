\newpage

# Introduction

[**pgvictoria**][pgvictoria] is a tuning solution for [PostgreSQL][postgresql].

[PostgreSQL][postgresql] ships with conservative default settings, so most installations end up
changing the configuration over time - for performance, for hardware, or for specific workloads.
After a while it becomes hard to answer a simple question: which settings have actually been
changed from their defaults ?

Knowing the answer matters when you are

* Auditing an installation
* Troubleshooting a performance problem
* Upgrading to a new PostgreSQL version
* Comparing environments

[**pgvictoria**][pgvictoria] answers that question by comparing a PostgreSQL configuration -
either from a live server or from a `postgresql.conf` file - against the out-of-the-box
default configuration for that PostgreSQL version.

[**pgvictoria**][pgvictoria] is named after the Roman Goddess of Speed.

## Features

* PostgreSQL configuration comparison
* Online reports against a live PostgreSQL instance
* Offline reports against a `postgresql.conf` file
* Text, HTML and Markdown report formats
* Configuration baselines for PostgreSQL 14 - 19
* Configuration generator

## Platforms

The supported platforms are

* [Fedora][fedora] 42+
* [Rocky Linux][rocky] 10.x
* [FreeBSD][freebsd]
