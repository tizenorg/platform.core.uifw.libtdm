Name:           libtdm
Version:        1.1.0
Release:        0
Summary:        User Library of Tizen Display Manager
Group:          Development/Libraries
License:        MIT
Source0:        %{name}-%{version}.tar.gz
Source1001:	    %{name}.manifest
BuildRequires:  pkgconfig(pthread-stubs)
BuildRequires:  pkgconfig(libtbm)
BuildRequires:  pkgconfig(libpng)
BuildRequires:  pkgconfig(ttrace)
BuildRequires:  pkgconfig(wayland-server)

%description
Common user library of Tizen Display Manager : libtdm front-end library

%package devel
Summary:        Devel of Tizen Display Manager Library
Group:          Development/Libraries
Requires:       libtdm = %{version}
Requires:       pkgconfig(libtbm)

%description devel
This supports frontend & backend library header and so

%global TZ_SYS_RO_SHARE  %{?TZ_SYS_RO_SHARE:%TZ_SYS_RO_SHARE}%{!?TZ_SYS_RO_SHARE:/usr/share}

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

%remove_docs

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig


%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{TZ_SYS_RO_SHARE}/license/%{name}
%{_libdir}/libtdm.so.*

%files devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/*
%{_libdir}/pkgconfig/*
%{_libdir}/libtdm.so

%changelog
