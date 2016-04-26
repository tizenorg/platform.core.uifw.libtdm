bin_PROGRAMS = \
	tdm-test-client

tdm_test_client_SOURCES = \
	tdm_test_client.c

tdm_test_client_LDFLAGS = ${LDFLAGS}
tdm_test_client_CFLAGS = \
	$(TDM_CFLAGS) \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/client
tdm_test_client_LDADD = \
	$(TDM_LIBS) \
	$(top_builddir)/client/libtdm-client.la
