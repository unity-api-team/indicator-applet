
libexec_PROGRAMS = \
	indicator-applet \
	indicator-applet-session \
	indicator-applet-complete

indicator_applet_CFLAGS = \
	-DG_LOG_DOMAIN=\""Indicator-Applet"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DINDICATOR_DIR=\""$(INDICATORDIR)"\" \
	-DINDICATOR_ICONS_DIR=\""$(INDICATORICONSDIR)"\" \
	-DINDICATOR_APPLET \
	-I$(srcdir)/.. \
	$(APPLET_CFLAGS)

indicator_applet_SOURCES = \
	applet-main.c

indicator_applet_LDADD = \
	$(APPLET_LIBS)

indicator_applet_session_CFLAGS = \
	-DG_LOG_DOMAIN=\""Indicator-Applet-Session"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DINDICATOR_DIR=\""$(INDICATORDIR)"\" \
	-DINDICATOR_ICONS_DIR=\""$(INDICATORICONSDIR)"\" \
	-DINDICATOR_APPLET_SESSION \
	-I$(srcdir)/.. \
	$(APPLET_CFLAGS)

indicator_applet_session_SOURCES = \
	applet-main.c

indicator_applet_session_LDADD = \
	$(APPLET_LIBS)

indicator_applet_complete_CFLAGS = \
	-DG_LOG_DOMAIN=\""Indicator-Applet-Complete"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DINDICATOR_DIR=\""$(INDICATORDIR)"\" \
	-DINDICATOR_ICONS_DIR=\""$(INDICATORICONSDIR)"\" \
	-DINDICATOR_APPLET_COMPLETE \
	-I$(srcdir)/.. \
	$(APPLET_CFLAGS)

indicator_applet_complete_SOURCES =	\
	applet-main.c

indicator_applet_complete_LDADD = \
	$(APPLET_LIBS)
