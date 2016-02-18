AC_PREREQ([2.60])
AC_INIT([libtdm],
        [1.1.0],
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

PKG_CHECK_MODULES(TDM, libtbm pthread-stubs libpng)
AC_SUBST(TDM_CFLAGS)
AC_SUBST(TDM_LIBS)

# set the dir for the tbm module
DEFAULT_TDM_MODULE_PATH="${libdir}/tdm"
AC_ARG_WITH(tdm-module-path, AS_HELP_STRING([--with-tdm-module-path=PATH], [tdm module dir]),
				[ TDM_MODULE_PATH="$withval" ],
				[ TDM_MODULE_PATH="${DEFAULT_TDM_MODULE_PATH}" ])
AC_DEFINE_UNQUOTED(TDM_MODULE_PATH, "${TDM_MODULE_PATH}", [Directory for the modules of tdm])

# For enumerating devices in test case
AC_SUBST(WARN_CFLAGS)
AC_OUTPUT([
	Makefile
	libtdm.pc
	include/Makefile
	src/Makefile])

echo ""
echo "$PACKAGE_STRING will be compiled with:"
echo ""
echo "TDM_CFLAGS        : $TDM_CFLAGS"
echo "TDM_LIBS          : $TDM_LIBS"
echo "TDM_MODULE_PATH   : $TDM_MODULE_PATH"
echo ""
