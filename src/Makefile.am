INCLUDES = \
	-DDATADIR=\""$(datadir)"\" \
	-DINDICATOR_DIR=\""$(libdir)/indicators/1"\"

libexec_PROGRAMS = \
	indicator-applet

indicator_applet_CFLAGS = \
	-DG_LOG_DOMAIN=\""Indicator-Applet"\" \
	$(APPLET_CFLAGS)

indicator_applet_SOURCES = 			\
	applet-main.c

indicator_applet_LDADD = \
	$(APPLET_LIBS)

awndir = $(AWNAPPLETDIR)
awn_LTLIBRARIES = \
	awn-indicator-applet.la

awn_indicator_applet_la_CFLAGS = \
	-DG_LOG_DOMAIN=\""Awn-Indicator-Applet"\" \
	$(AWN_APPLET_CFLAGS)

awn_indicator_applet_la_SOURCES = 			\
	awn-applet-main.c

awn_indicator_applet_la_LIBADD = \
	$(AWN_APPLET_LIBS)

awn_indicator_applet_la_LDFLAGS = -module -avoid-version
