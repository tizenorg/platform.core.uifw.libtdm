Name:           libtdm
Version:        1.3.0
Release:        0
Summary:        User Library of Tizen Display Manager
Group:          Development/Libraries
License:        MIT
Source0:        %{name}-%{version}.tar.gz

%description
Common user library of Tizen Display Manager : libtdm front-end library

%prep
# our %{name}-%{version}.tar.gz archive hasn't top-level directory,
# so we create it
%setup -q -c %{name}-%{version}

%build
PKG_CONFIG_PATH=/usr/local/lib/pkgconfig \
./autogen.sh --build=x86_64-unknown-linux-gnu \
	     --host=arm-linux-androideabi \
	     --disable-static \
	     --enable-android-support \
	     CFLAGS="${CFLAGS} -Wall" \
	     LDFLAGS="${LDFLAGS} -Wl,--hash-style=both -Wl,--as-needed"

make %{?_smp_mflags}

%install
rm -rf %{buildroot}

%make_install

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
/usr/local/*
