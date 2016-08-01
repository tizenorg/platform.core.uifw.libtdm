libtdm_la_LTLIBRARIES = libtdm.la
libtdm_ladir = $(libdir)
libtdm_la_LDFLAGS = -version-number 1:0:0 -no-undefined
libtdm_la_LIBADD = $(TDM_LIBS) -ldl ../common/libtdm-common.la

# the Android's libc (bionic) has the built-in support for pthread
if !ANDROID_SUPPORT
	libtdm_la_LIBADD += -lpthread
endif

libtdm_la_CFLAGS = \
	$(CFLAGS) \
	$(WARN_CFLAGS) \
	$(TDM_CFLAGS) \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/protocol \
	-I$(top_srcdir)/src

libtdm_la_SOURCES = \
	$(top_srcdir)/protocol/tdm-protocol.c \
	tdm_backend.c \
	tdm_server.c \
	tdm_vblank.c \
	tdm_event_loop.c \
	tdm_thread.c \
	tdm_helper.c \
	tdm_buffer.c \
	tdm_display.c \
	tdm_pp.c \
	tdm_capture.c \
	tdm_dbg_server.c \
	tdm.c
