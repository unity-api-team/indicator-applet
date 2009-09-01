
libexec_PROGRAMS = \
	indicator-applet

indicator_applet_CFLAGS = \
	-DG_LOG_DOMAIN=\""Indicator-Applet"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DINDICATOR_DIR=\""$(INDICATORDIR)"\" \
	-DINDICATOR_ICONS_DIR=\""$(INDICATORICONSDIR)"\" \
	-I$(srcdir)/.. \
	$(APPLET_CFLAGS)

indicator_applet_SOURCES = 			\
	applet-main.c

indicator_applet_LDADD = \
	$(APPLET_LIBS)
