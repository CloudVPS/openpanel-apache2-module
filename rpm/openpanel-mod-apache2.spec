%define version 0.9.5

%define libpath /usr/lib
%ifarch x86_64
  %define libpath /usr/lib64
%endif

Summary: Configure the apache2 webserver
Name: openpanel-mod-apache2
Version: %version
Release: 1
License: GPLv2
Group: Development
Source: http://packages.openpanel.com/archive/openpanel-mod-apache2-%{version}.tar.gz
Patch1: openpanel-mod-apache2-00-makefile
BuildRoot: /var/tmp/%{name}-buildroot
Requires: openpanel-core >= 0.8.3
Requires: openpanel-mod-user
Requires: openpanel-mod-domain
Requires: httpd
Requires: logax
Requires: mod_perl
Requires: php

%description
Configure the apache2 webserver
Openpanel webserver management module

%prep
%setup -q -n openpanel-mod-apache2-%version
%patch1 -p0 -b .buildroot

%build
BUILD_ROOT=$RPM_BUILD_ROOT
./configure
make

%install
BUILD_ROOT=$RPM_BUILD_ROOT
rm -rf ${BUILD_ROOT}
mkdir -p ${BUILD_ROOT}/var/opencore/modules/Apache2.module
mkdir -p ${BUILD_ROOT}/var/opencore/conf/staging/Apache2
mkdir -p ${BUILD_ROOT}/etc/httpd/conf.d
mkdir -p ${BUILD_ROOT}/var/log/httpd/openpanel/logs
cp -rf ./apache2module.app ${BUILD_ROOT}/var/opencore/modules/Apache2.module/
ln -sf ./apache2module.app/exec ${BUILD_ROOT}/var/opencore/modules/Apache2.module/action
cp module.xml *.html techsupport.* ${BUILD_ROOT}/var/opencore/modules/Apache2.module/
install -m 755 verify ${BUILD_ROOT}/var/opencore/modules/Apache2.module/verify
install -m 644 REDHAT/openpanel.conf ${BUILD_ROOT}/etc/httpd/conf.d/openpanel.conf

%post
chown opencore:authd /var/opencore/conf/staging/Apache2
mkdir -p /etc/httpd/openpanel.d
chkconfig --level 2345 httpd on
rm -f /etc/httpd/conf.d/welcome.conf
service httpd restart >/dev/null 2>&1

%files
%defattr(-,root,root)
/