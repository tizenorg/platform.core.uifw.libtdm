AM_CFLAGS = \
	$(TDM_CFLAGS) \
	-I$(top_srcdir)/include

AM_LDFLAGS = \
	$(TDM_LIBS) \
	$(top_builddir)/src/libtdm.la

bin_PROGRAMS = \
	tdm-test

tdm_test_SOURCES = \
	tdm-test.c \
	tdm-test-display.c \
	tdm-test-output.c \
	tdm-test-pp.c \
	tdm-test-capture.c \
	tdm-test-buffer.c
