
libexec_PROGRAMS = \
	indicator-applet-session

indicator_applet_session_CFLAGS = \
	-DG_LOG_DOMAIN=\""Indicator-Applet"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DINDICATOR_DIR=\""$(INDICATORDIR)"\" \
	-DINDICATOR_ICONS_DIR=\""$(INDICATORICONSDIR)"\" \
	-I$(srcdir)/.. \
	$(APPLET_CFLAGS)

indicator_applet_session_SOURCES = 			\
	applet-main.c

indicator_applet_session_LDADD = \
	$(APPLET_LIBS)
