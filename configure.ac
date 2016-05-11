AC_PREREQ([2.60])

m4_define([tdm_major_version],  [1])
m4_define([tdm_minor_version],  [2])
m4_define([tdm_micro_version],  [0])
m4_define([tdm_version],
          [tdm_major_version.tdm_minor_version.tdm_micro_version])

AC_INIT([libtdm],
        [tdm_version],
        [https://www.tizen.org],
        [libtdm])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([Makefile.am])
AM_INIT_AUTOMAKE([1.10 foreign dist-bzip2])
AM_MAINTAINER_MODE([enable])

# Check for programs
AC_PROG_CC

AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE
AC_FUNC_ALLOCA

# Initialize libtool
LT_PREREQ([2.2])
LT_INIT([disable-static])

# Enable quiet compiles on automake 1.11.
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_PATH_PROG([wayland_scanner], [wayland-scanner])
if test x$wayland_scanner = x; then
   AC_MSG_ERROR([wayland-scanner is needed to compile])
fi

PKG_CHECK_MODULES(WAYLAND_SCANNER, wayland-scanner >= 1.7.0)

PKG_CHECK_MODULES(TDM, dlog libtbm pthread-stubs libpng wayland-server)

PKG_CHECK_MODULES(TTRACE,
		[ttrace],
		[have_ttrace="yes"], [have_ttrace="no"])
if test "x$have_ttrace" = "xyes"; then
    TDM_CFLAGS="$TDM_CFLAGS $TTRACE_CFLAGS"
    TDM_LIBS="$TDM_LIBS $TTRACE_LIBS"
	AC_DEFINE(HAVE_TTRACE, 1, [ttrace available])
fi

AC_SUBST(TDM_CFLAGS)
AC_SUBST(TDM_LIBS)

PKG_CHECK_MODULES(TDM_CLIENT, dlog wayland-client)
AC_SUBST(TDM_CLIENT_CFLAGS)
AC_SUBST(TDM_CLIENT_LIBS)

# set the dir for the tbm module
DEFAULT_TDM_MODULE_PATH="${libdir}/tdm"
AC_ARG_WITH(tdm-module-path, AS_HELP_STRING([--with-tdm-module-path=PATH], [tdm module dir]),
				[ TDM_MODULE_PATH="$withval" ],
				[ TDM_MODULE_PATH="${DEFAULT_TDM_MODULE_PATH}" ])
AC_DEFINE_UNQUOTED(TDM_MODULE_PATH, "${TDM_MODULE_PATH}", [Directory for the modules of tdm])

AC_SUBST([TDM_MAJOR_VERSION], [tdm_major_version])
AC_SUBST([TDM_MINOR_VERSION], [tdm_minor_version])
AC_SUBST([TDM_MICRO_VERSION], [tdm_micro_version])
AC_SUBST([TDM_VERSION], [tdm_version])

AC_DEFINE([TDM_MAJOR_VERSION], [tdm_major_version], [tdm ABI major version])
AC_DEFINE([TDM_MINOR_VERSION], [tdm_minor_version], [tdm ABI minor version])
AC_DEFINE([TDM_MICRO_VERSION], [tdm_micro_version], [tdm ABI macro version])

# For enumerating devices in test case
AC_SUBST(WARN_CFLAGS)
AC_OUTPUT([
	Makefile
	libtdm.pc
	doc/tdm_doc.h
	include/Makefile
	protocol/Makefile
	client/libtdm-client.pc
	client/Makefile
	src/Makefile
	tools/Makefile])

echo ""
echo "$PACKAGE_STRING will be compiled with:"
echo ""
echo "TDM_CFLAGS        : $TDM_CFLAGS"
echo "TDM_LIBS          : $TDM_LIBS"
echo "TDM_CLIENT_CFLAGS : $TDM_CLIENT_CFLAGS"
echo "TDM_CLIENT_LIBS   : $TDM_CLIENT_LIBS"
echo "TDM_MODULE_PATH   : $TDM_MODULE_PATH"
echo ""
