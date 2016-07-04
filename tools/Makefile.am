bin_PROGRAMS = \
	tdm-test-server \
	tdm-test-client

#tdm-test-server
tdm_test_server_SOURCES = \
	buffers.c \
	tdm_test_server.c
tdm_test_server_LDFLAGS = ${LDFLAGS}
tdm_test_server_CFLAGS = \
	$(TDM_CFLAGS) \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src
tdm_test_server_LDADD = \
	$(TDM_LIBS) \
	$(top_builddir)/src/libtdm.la

#tdm-test-cliet
tdm_test_client_SOURCES = \
	tdm_test_client.c
tdm_test_client_LDFLAGS = ${LDFLAGS}
tdm_test_client_CFLAGS = \
	$(TDM_CFLAGS) \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/client \
	-I$(top_srcdir)/src
tdm_test_client_LDADD = \
	$(TDM_LIBS) \
	$(top_builddir)/client/libtdm-client.la
