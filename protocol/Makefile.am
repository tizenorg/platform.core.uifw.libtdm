### protocol.[ch]
%-protocol.c : %.xml
	$(wayland_scanner) code < $< > $@
%-server-protocol.h : %.xml
	$(wayland_scanner) server-header < $< > $@
%-client-protocol.h : %.xml
	$(wayland_scanner) client-header < $< > $@

BUILT_SOURCES = \
	tdm-server-protocol.h \
	tdm-client-protocol.h \
	tdm-protocol.c

CLEANFILES = $(BUILT_SOURCES)
