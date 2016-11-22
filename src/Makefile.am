indicator_applet_libdir = ${pkglibdir}
indicator_applet_lib_LTLIBRARIES = \
	libindicator-applet.la \
	libindicator-applet-appmenu.la \
	libindicator-applet-session.la \
	libindicator-applet-complete.la

APPLET_SOURCES = \
	applet-main.c \
	eggaccelerators.c \
	eggaccelerators.h \
	tomboykeybinder.c \
	tomboykeybinder.h

APPLET_CPPFLAGS = \
	-DDATADIR=\""$(datadir)"\" \
	-DINDICATOR_DIR=\""$(INDICATORDIR)"\" \
	-DINDICATOR_ICONS_DIR=\""$(INDICATORICONSDIR)"\" \
	-DGDK_DISABLE_DEPRECATED \
	-I$(srcdir)/..

AM_CFLAGS = $(APPLET_CFLAGS)
AM_LDFLAGS = -module -avoid-version

libindicator_applet_la_SOURCES = $(APPLET_SOURCES)
libindicator_applet_la_CPPFLAGS = $(APPLET_CPPFLAGS) \
	-DG_LOG_DOMAIN=\""Indicator-Applet"\" \
	-DINDICATOR_APPLET
libindicator_applet_la_LIBADD = $(APPLET_LIBS)

libindicator_applet_appmenu_la_SOURCES = $(APPLET_SOURCES)
libindicator_applet_appmenu_la_CPPFLAGS = $(APPLET_CPPFLAGS) \
	-DG_LOG_DOMAIN=\""Indicator-Applet-Appmenu"\" \
	-DINDICATOR_APPLET_APPMENU
libindicator_applet_appmenu_la_LIBADD = $(APPLET_LIBS)

libindicator_applet_session_la_SOURCES = $(APPLET_SOURCES)
libindicator_applet_session_la_CPPFLAGS = $(APPLET_CPPFLAGS) \
	-DG_LOG_DOMAIN=\""Indicator-Applet-Session"\" \
	-DINDICATOR_APPLET_SESSION
libindicator_applet_session_la_LIBADD = $(APPLET_LIBS)

libindicator_applet_complete_la_SOURCES = $(APPLET_SOURCES)
libindicator_applet_complete_la_CPPFLAGS = $(APPLET_CPPFLAGS) \
	-DG_LOG_DOMAIN=\""Indicator-Applet-Complete"\" \
	-DINDICATOR_APPLET_COMPLETE
libindicator_applet_complete_la_LIBADD = $(APPLET_LIBS)
