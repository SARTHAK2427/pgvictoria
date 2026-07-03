\newpage

# Installation

## Rocky Linux 10.x

We can download the [Rocky Linux](https://www.rockylinux.org/) distribution from their web site

```
https://rockylinux.org/download
```

The installation and setup is beyond the scope of this guide.

Ideally, you would use dedicated user accounts to run [**PostgreSQL**][postgresql] and [**pgvictoria**][pgvictoria]

```
useradd postgres
usermod -a -G wheel postgres
useradd pgvictoria
usermod -a -G wheel pgvictoria
```

Add a configuration directory for [**pgvictoria**][pgvictoria]

```
mkdir /etc/pgvictoria
chown -R pgvictoria:pgvictoria /etc/pgvictoria
```

## PostgreSQL 18

We will install PostgreSQL 18 from the official [YUM repository][yum] with the community binaries,

**x86_64**

```
dnf -qy module disable postgresql
dnf install -y https://download.postgresql.org/pub/repos/yum/reporpms/EL-10-x86_64/pgdg-redhat-repo-latest.noarch.rpm
```

**aarch64**

```
dnf -qy module disable postgresql
dnf install -y https://download.postgresql.org/pub/repos/yum/reporpms/EL-10-aarch64/pgdg-redhat-repo-latest.noarch.rpm
```

and do the install via

```
dnf install -y postgresql18 postgresql18-server postgresql18-contrib
```

First, we will update `~/.bashrc` with

```
cat >> ~/.bashrc
export PGHOST=/tmp
export PATH=/usr/pgsql-18/bin/:$PATH
```

then Ctrl-d to save, and

```
source ~/.bashrc
```

to reload the Bash environment.

Then we can do the PostgreSQL initialization

```
mkdir DB
initdb -k DB
```

Please, check with other sources in order to create a setup for your local setup.

Now, we are ready to start PostgreSQL

```
pg_ctl -D DB -l /tmp/ start
```

## pgvictoria

[**pgvictoria**][pgvictoria] requires

* [clang][clang]
* [cmake][cmake]
* [make][make]
* [libev][libev]
* [OpenSSL][openssl]
* [libxml2][libxml2]
* [rst2man][rst2man]
* [pandoc][pandoc]
* [texlive][texlive]

```
dnf install git gcc clang clang-analyzer clang-tools-extra cmake make libev libev-devel \
    openssl openssl-devel libxml2 libxml2-devel python3-docutils libatomic \
    libasan libasan-static
```

Alternative [gcc][gcc] can be used.

The following commands will install [**pgvictoria**][pgvictoria] in the `/usr/local` hierarchy.

```
git clone https://github.com/pgvictoria/pgvictoria.git
cd pgvictoria
mkdir build
cd build
cmake -DCMAKE_C_COMPILER=clang -DCMAKE_INSTALL_PREFIX=/usr/local ..
make
sudo make install
```
