%define 	modname	Apache2

Name: 		openpanel-mod-apache2
Version: 	1.0.2
Release: 	1%{?dist}
Summary:  	OpenPanel module to manage Apache2
License: 	GPLv3
Group: 		Applications/Internet
Source: 	%{name}-%{version}.tar.bz2
Requires:	openpanel-core
Requires: 	httpd >= 2.0
Requires: 	php >= 5.1
Requires:	openpanel-logax
BuildRequires:	openpanel-core-devel
BuildRequires:	grace-devel
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

%description
OpenPanel module to manage Apache2

%prep
%setup -q -n %{modname}.module
./configure --prefix=%{_prefix} --exec-prefix=%{_bindir} \
            --lib-prefix=%{_libdir} --conf-prefix=%{_sysconfdir} \
	    --include-prefix=%{_includedir}

%build
make

%install
rm -rf %{buildroot}
%makeinstall DESTDIR=%{buildroot}
mkdir -p %{buildroot}/%{_sysconfdir}/httpd/conf.d/
mkdir -p %{buildroot}/%{_sysconfdir}/httpd/openpanel.d
mkdir -p %{buildroot}/%{_localstatedir}/log/httpd/openpanel
install -m 644 REDHAT/openpanel.conf %{buildroot}/%{_sysconfdir}/httpd/conf.d/openpanel.conf

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%dir %attr(-,openpanel-core, openpanel-authd) %{_localstatedir}/openpanel/conf/staging/%{modname}
%config %attr(0644,root,root) %{_sysconfdir}/httpd/conf.d/openpanel.conf
%dir %attr(0755,root,root) %{_sysconfdir}/httpd/openpanel.d/
%dir %attr(0755,root,root) %{_localstatedir}/log/httpd/openpanel/
%attr(-,openpanel-core, openpanel-authd)%{_localstatedir}/openpanel/modules/%{modname}.module

%post
/sbin/service openpaneld condrestart /dev/null 2>&1

%preun
if [ $1 = 0 ]; then
	service openpaneld stop >/dev/null 2>&1
fi

%postun
if [ $1 = 0 ]; then
	service openpaneld start >/dev/null 2>&1
fi
if [ "$1" = "1" ]; then
	service openpaneld condrestart >/dev/null 2>&1
fi

%changelog
* Wed Jan 18 2011 Igmar Palsenberg <igmar@palsenberg.com>
- Initial packaging
