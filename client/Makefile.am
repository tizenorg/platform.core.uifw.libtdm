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
### tdm-monitor
##########################################################################
bin_PROGRAMS = \
	tdm-monitor

#tdm-monitor
tdm_monitor_SOURCES = \
	$(top_srcdir)/protocol/tdm-protocol.c \
	tdm_monitor.c
tdm_monitor_LDFLAGS = ${LDFLAGS}
tdm_monitor_LDADD = $(TDM_CLIENT_LIBS) ../common/libtdm-common.la
tdm_monitor_CFLAGS = \
	$(TDM_CFLAGS) \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/protocol \
	-I$(top_srcdir)/src
