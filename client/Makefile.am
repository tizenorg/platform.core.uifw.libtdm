##########################################################################
### tdm-client library
##########################################################################
libtdm_clientincludedir = ${includedir}
libtdm_clientinclude_HEADERS = \
	tdm_client.h \
	tdm_client_types.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libtdm-client.pc

libtdm_client_la_LTLIBRARIES = libtdm-client.la
libtdm_client_ladir = $(libdir)
libtdm_client_la_LDFLAGS = $(LDFLAGS)
libtdm_client_la_LIBADD = $(TDM_CLIENT_LIBS) ../common/libtdm-common.la
libtdm_client_la_CFLAGS = \
	$(CFLAGS) \
	$(WARN_CFLAGS) \
	$(TDM_CLIENT_CFLAGS) \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/protocol \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/client

libtdm_client_la_SOURCES = \
	$(top_srcdir)/protocol/tdm-protocol.c \
	tdm_client.c

##########################################################################
### tdm-dbg
##########################################################################
bin_PROGRAMS = \
	tdm-dbg

#tdm-dbg
tdm_dbg_SOURCES = \
	$(top_srcdir)/protocol/tdm-protocol.c \
	tdm_dbg.c
tdm_dbg_LDFLAGS = ${LDFLAGS}
tdm_dbg_LDADD = $(TDM_CLIENT_LIBS) ../common/libtdm-common.la
tdm_dbg_CFLAGS = \
	$(TDM_CFLAGS) \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/protocol \
	-I$(top_srcdir)/src
