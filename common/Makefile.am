noinst_LTLIBRARIES = libtdm-common.la

libtdm_common_la_SOURCES = tdm_log.c
libtdm_common_la_LIBADD = ${LDFLAGS} $(TDM_LDFLAGS)
libtdm_common_la_LDFLAGS = ${LDFLAGS} $(TDM_LDFLAGS)
libtdm_common_la_CFLAGS = \
	$(CFLAGS) \
	$(WARN_CFLAGS) \
	$(TDM_CFLAGS) \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/protocol \
	-I$(top_srcdir)/src
