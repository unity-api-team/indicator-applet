# This file will be processed with automake to create Makefile.in

##############################
# Bonobo .server
##############################

serverdir =					\
	$(libdir)/bonobo/servers
server_in_files =				\
	GNOME_IndicatorApplet.server.in \
	GNOME_IndicatorAppletComplete.server.in \
	GNOME_IndicatorAppletAppmenu.server.in \
	GNOME_FastUserSwitchApplet.server.in
server_DATA =					\
	$(server_in_files:.server.in=.server)

%.server.in: %.server.in.in
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" -e "s|\@VERSION\@|$(VERSION)|" $< > $@

server_in_in_files = \
	$(server_in_files:.server.in=.server.in.in)

@INTLTOOL_SERVER_RULE@


##############################
# Icons
##############################

iconsdir = $(datadir)/icons/hicolor/scalable/apps

icons_DATA = indicator-applet.svg

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache
update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated.  After (un)install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi


##############################
# Autojunk
##############################

CLEANFILES =					\
	$(server_in_files)			\
	$(server_DATA)				\
	$(schema_DATA)				\
	*.bak

EXTRA_DIST =					\
	$(icons_DATA) \
	$(server_in_in_files) \
	$(schema_in_files)

MAINTAINERCLEANFILES =				\
	Makefile.in


