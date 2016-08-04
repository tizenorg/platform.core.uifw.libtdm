Name:           libtdm
Version:        1.3.1
Release:        0
Summary:        User Library of Tizen Display Manager
Group:          Development/Libraries
License:        MIT
Source0:        %{name}-%{version}.tar.gz
Source1001:	    %{name}.manifest
BuildRequires:  pkgconfig(libtbm)
BuildRequires:  pkgconfig(libpng)
BuildRequires:  pkgconfig(ttrace)
BuildRequires:  pkgconfig(wayland-server)
BuildRequires:  pkgconfig(pixman-1)

%description
Common user library of Tizen Display Manager : libtdm front-end library

%package devel
Summary:        Devel of Tizen Display Manager Library
Group:          Development/Libraries
Requires:       libtdm = %{version}
Requires:       pkgconfig(libtbm)

%description devel
This supports frontend & backend library header and so

%package client
Summary:        Client library for Tizen Display Manager
Group:          Development/Libraries
Requires:       libtdm = %{version}

%description client
Tizen Display Manager Client Library

%package client-devel
Summary:        Client library for Tizen Display Manager
Group:          Development/Libraries
Requires:       libtdm-client = %{version}
Requires:       libtdm-devel

%description client-devel
Tizen Display Manager Client Library headers

%global TZ_SYS_RO_SHARE  %{?TZ_SYS_RO_SHARE:%TZ_SYS_RO_SHARE}%{!?TZ_SYS_RO_SHARE:/usr/share}

%package tools
Summary:        Tools for libtdm
Group:          Development/Utilities
Requires:       libtdm = %{version}

%description tools
This contains libtdm tools for fundamental testing

%prep
%setup -q
cp %{SOURCE1001} .

%build
%reconfigure --disable-static \
             CFLAGS="${CFLAGS} -Wall -Werror" \
             LDFLAGS="${LDFLAGS} -Wl,--hash-style=both -Wl,--as-needed"
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/%{TZ_SYS_RO_SHARE}/license
cp -af COPYING %{buildroot}/%{TZ_SYS_RO_SHARE}/license/%{name}
%make_install

%__mkdir_p %{buildroot}%{_unitdir}
install -m 644 service/tdm-socket.service %{buildroot}%{_unitdir}
install -m 644 service/tdm-socket.path %{buildroot}%{_unitdir}
%__mkdir_p %{buildroot}%{_unitdir_user}
install -m 644 service/tdm-socket-user.service %{buildroot}%{_unitdir_user}
install -m 644 service/tdm-socket-user.path %{buildroot}%{_unitdir_user}

%remove_docs


%pre
%__mkdir_p %{_unitdir}/graphical.target.wants
ln -sf ../tdm-socket.path %{_unitdir}/graphical.target.wants/
%__mkdir_p %{_unitdir_user}/default.target.wants
ln -sf ../tdm-socket-user.path %{_unitdir_user}/default.target.wants/

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig
rm -f %{_unitdir}/graphical.target.wants/tdm-socket.path
rm -f %{_unitdir_user}/default.target.wants/tdm-socket-user.path

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{TZ_SYS_RO_SHARE}/license/%{name}
%{_libdir}/libtdm.so.*
%attr(750,root,root) %{_bindir}/tdm-monitor
%{_unitdir}/tdm-socket.path
%{_unitdir}/tdm-socket.service
%{_unitdir_user}/tdm-socket-user.path
%{_unitdir_user}/tdm-socket-user.service

%files devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/tdm.h
%{_includedir}/tdm_common.h
%{_includedir}/tdm_backend.h
%{_includedir}/tdm_helper.h
%{_includedir}/tdm_list.h
%{_includedir}/tdm_log.h
%{_includedir}/tdm_types.h
%{_libdir}/pkgconfig/libtdm.pc
%{_libdir}/libtdm.so

%files client
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{TZ_SYS_RO_SHARE}/license/%{name}
%{_libdir}/libtdm-client.so.*

%files client-devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/tdm_client.h
%{_includedir}/tdm_client_types.h
%{_libdir}/pkgconfig/libtdm-client.pc
%{_libdir}/libtdm-client.so

%files tools
%manifest %{name}.manifest
%attr(750,root,root) %{_bindir}/tdm-test-server
%{_bindir}/tdm-test-client

%changelog
