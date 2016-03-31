libtdm_clientincludedir = ${includedir}
libtdm_clientinclude_HEADERS = \
	tdm_client.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libtdm-client.pc

libtdm_client_la_LTLIBRARIES = libtdm-client.la
libtdm_client_ladir = $(libdir)
libtdm_client_la_LDFLAGS = $(LDFLAGS)
libtdm_client_la_LIBADD = $(TDM_CLIENT_LIBS)
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
