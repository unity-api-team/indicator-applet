/*
A small wrapper utility to load indicators and put them as menu items
into the gnome-panel using it's applet interface.

Copyright 2009 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>

This program is free software: you can redistribute it and/or modify it 
under the terms of the GNU General Public License version 3, as published 
by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranties of 
MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR 
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along 
with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <config.h>
#include <panel-applet.h>

#include "libindicator/indicator-object.h"

#define ENTRY_DATA_NAME "indicator-custom-entry-data"

static gboolean     applet_fill_cb (PanelApplet * applet, const gchar * iid, gpointer data);


static void cw_panel_background_changed (PanelApplet               *applet,
                                         PanelAppletBackgroundType  type,
                        				         GdkColor                  *colour,
                        				         GdkPixmap                 *pixmap,
                                         GtkWidget                 *menubar);

/*************
 * main
 * ***********/

#ifdef INDICATOR_APPLET
PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_IndicatorApplet_Factory",
               PANEL_TYPE_APPLET,
               "indicator-applet", "0",
               applet_fill_cb, NULL);
#endif
#ifdef INDICATOR_APPLET_SESSION
PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_FastUserSwitchApplet_Factory",
               PANEL_TYPE_APPLET,
               "indicator-applet-session", "0",
               applet_fill_cb, NULL);
#endif
#ifdef INDICATOR_APPLET_COMPLETE
PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_IndicatorAppletComplete_Factory",
               PANEL_TYPE_APPLET,
               "indicator-applet-complete", "0",
               applet_fill_cb, NULL);
#endif

/*************
 * init function
 * ***********/
static void
entry_added (IndicatorObject * io, IndicatorObjectEntry * entry, GtkWidget * menu)
{
	g_debug("Signal: Entry Added");

	GtkWidget * menuitem = gtk_menu_item_new();
	GtkWidget * hbox = gtk_hbox_new(FALSE, 3);

	if (entry->image != NULL) {
		gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(entry->image), FALSE, FALSE, 0);
	}
	if (entry->label != NULL) {
		gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(entry->label), FALSE, FALSE, 0);
	}
	gtk_container_add(GTK_CONTAINER(menuitem), hbox);
	gtk_widget_show(hbox);

	if (entry->menu != NULL) {
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), GTK_WIDGET(entry->menu));
	}

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	gtk_widget_show(menuitem);

	g_object_set_data(G_OBJECT(menuitem), ENTRY_DATA_NAME, entry);

	return;
}

static void
entry_removed_cb (GtkWidget * widget, gpointer userdata)
{
	gpointer data = g_object_get_data(G_OBJECT(widget), ENTRY_DATA_NAME);

	if (data != userdata) {
		return;
	}

	gtk_widget_destroy(widget);
	return;
}

static void 
entry_removed (IndicatorObject * io, IndicatorObjectEntry * entry, gpointer user_data)
{
	g_debug("Signal: Entry Removed");

	gtk_container_foreach(GTK_CONTAINER(user_data), entry_removed_cb, entry);

	return;
}

static gboolean
load_module (const gchar * name, GtkWidget * menu)
{
	g_debug("Looking at Module: %s", name);
	g_return_val_if_fail(name != NULL, FALSE);

	if (!g_str_has_suffix(name, G_MODULE_SUFFIX)) {
		return FALSE;
	}

	g_debug("Loading Module: %s", name);

	/* Build the object for the module */
	gchar * fullpath = g_build_filename(INDICATOR_DIR, name, NULL);
	IndicatorObject * io = indicator_object_new_from_file(fullpath);
	g_free(fullpath);

	/* Connect to it's signals */
	g_signal_connect(G_OBJECT(io), INDICATOR_OBJECT_SIGNAL_ENTRY_ADDED,   G_CALLBACK(entry_added),    menu);
	g_signal_connect(G_OBJECT(io), INDICATOR_OBJECT_SIGNAL_ENTRY_REMOVED, G_CALLBACK(entry_removed),  menu);

	/* Work on the entries */
	GList * entries = indicator_object_get_entries(io);
	GList * entry = NULL;

	for (entry = entries; entry != NULL; entry = g_list_next(entry)) {
		IndicatorObjectEntry * entrydata = (IndicatorObjectEntry *)entry->data;
		entry_added(io, entrydata, menu);
	}

	g_list_free(entries);

	return TRUE;
}

static gboolean
menubar_press (GtkWidget * widget,
                    GdkEventButton *event,
                    gpointer data)
{
	if (event->button != 1) {
		g_signal_stop_emission_by_name(widget, "button-press-event");
	}

	return FALSE;
}

static gboolean
menubar_on_expose (GtkWidget * widget,
                    GdkEventExpose *event,
                    GtkWidget * menubar)
{
	if (GTK_WIDGET_HAS_FOCUS(menubar))
		gtk_paint_focus(widget->style, widget->window, GTK_WIDGET_STATE(menubar),
		                NULL, widget, "menubar-applet", 0, 0, -1, -1);

	return FALSE;
}

static void
about_cb (BonoboUIComponent *ui_container,
	  gpointer           data,
	  const gchar       *cname)
{
	static const gchar *authors[] = {
		"Ted Gould <ted@canonical.com>",
		NULL
	};

	static gchar *license[] = {
        N_("This program is free software: you can redistribute it and/or modify it "
           "under the terms of the GNU General Public License version 3, as published "
           "by the Free Software Foundation."),
        N_("This program is distributed in the hope that it will be useful, but "
           "WITHOUT ANY WARRANTY; without even the implied warranties of "
           "MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR "
           "PURPOSE.  See the GNU General Public License for more details."),
        N_("You should have received a copy of the GNU General Public License along "
           "with this program.  If not, see <http://www.gnu.org/licenses/>."),
		NULL
	};
	gchar *license_i18n;

	license_i18n = g_strconcat (_(license[0]), "\n\n", _(license[1]), "\n\n", _(license[2]), NULL);

	gtk_show_about_dialog(NULL,
		"version", VERSION,
		"copyright", "Copyright \xc2\xa9 2009 Canonical, Ltd.",
#ifdef INDICATOR_APPLET_SESSION
		"comments", _("A place to adjust your status, change users or exit your session."),
#else
		"comments", _("An applet to hold all of the system indicators."),
#endif
		"authors", authors,
		"license", license_i18n,
		"wrap-license", TRUE,
		"translator-credits", _("translator-credits"),
		"logo-icon-name", "indicator-applet",
		"icon-name", "indicator-applet",
		"website", "http://launchpad.net/indicator-applet",
		"website-label", _("Indicator Applet Website"),
		NULL
	);

	g_free (license_i18n);

	return;
}

#ifdef N_
#undef N_
#endif
#define N_(x) x

static gboolean
applet_fill_cb (PanelApplet * applet, const gchar * iid, gpointer data)
{
	static const BonoboUIVerb menu_verbs[] = {
		BONOBO_UI_VERB ("IndicatorAppletAbout", about_cb),
		BONOBO_UI_VERB_END
	};
	static const gchar * menu_xml = 
		"<popup name=\"button3\">"
			"<menuitem name=\"About Item\" verb=\"IndicatorAppletAbout\" _label=\"" N_("_About") "\" pixtype=\"stock\" pixname=\"gtk-about\"/>"
		"</popup>";

	GtkWidget *menubar;
	gint i;
	gint indicators_loaded = 0;

	/* check if we are running stracciatella session */
	if (g_strcmp0(g_getenv("GDMSESSION"), "gnome-stracciatella") == 0) {
		g_debug("Running stracciatella GNOME session, disabling myself");
		return TRUE;
	}
  
	static gboolean first_time = FALSE;

#ifdef INDICATOR_APPLET_SESSION
	/* check if we are running stracciatella session */
	if (g_strcmp0(g_getenv("GDMSESSION"), "gnome-stracciatella") == 0) {
		g_debug("Running stracciatella GNOME session, disabling myself");
		return TRUE;
	}
#endif

	if (!first_time)
	{
		first_time = TRUE;
#ifdef INDICATOR_APPLET
		g_set_application_name(_("Indicator Applet"));
#endif
#ifdef INDICATOR_APPLET_SESSION
		g_set_application_name(_("Indicator Applet Session"));
#endif
#ifdef INDICATOR_APPLET_COMPLETE
		g_set_application_name(_("Indicator Applet Complete"));
#endif
	}

	/* Set panel options */
	gtk_container_set_border_width(GTK_CONTAINER (applet), 0);
	panel_applet_set_flags(applet, PANEL_APPLET_EXPAND_MINOR);
	panel_applet_setup_menu(applet, menu_xml, menu_verbs, NULL);
#ifdef INDICATOR_APPLET
	atk_object_set_name (gtk_widget_get_accessible (GTK_WIDGET (applet)),
	                     "indicator-applet");
#endif
#ifdef INDICATOR_APPLET_SESSION
	atk_object_set_name (gtk_widget_get_accessible (GTK_WIDGET (applet)),
	                     "indicator-applet-session");
#endif
#ifdef INDICATOR_APPLET_COMPLETE
	atk_object_set_name (gtk_widget_get_accessible (GTK_WIDGET (applet)),
	                     "indicator-applet-complete");
#endif
  
	/* Init some theme/icon stuff */
	gtk_icon_theme_append_search_path(gtk_icon_theme_get_default(),
	                                  INDICATOR_ICONS_DIR);
	/* g_debug("Icons directory: %s", INDICATOR_ICONS_DIR); */
	gtk_rc_parse_string (
	    "style \"indicator-applet-style\"\n"
        "{\n"
        "    GtkMenuBar::shadow-type = none\n"
        "    GtkMenuBar::internal-padding = 0\n"
        "    GtkWidget::focus-line-width = 0\n"
        "    GtkWidget::focus-padding = 0\n"
        "}\n"
	    "style \"indicator-applet-menubar-style\"\n"
        "{\n"
        "    GtkMenuBar::shadow-type = none\n"
        "    GtkMenuBar::internal-padding = 0\n"
        "    GtkWidget::focus-line-width = 0\n"
        "    GtkWidget::focus-padding = 0\n"
        "    GtkMenuItem::horizontal-padding = 0\n"
        "}\n"
	    "style \"indicator-applet-menuitem-style\"\n"
        "{\n"
        "    GtkWidget::focus-line-width = 0\n"
        "    GtkWidget::focus-padding = 0\n"
        "    GtkMenuItem::horizontal-padding = 0\n"
        "}\n"
        "widget \"*.fast-user-switch-applet\" style \"indicator-applet-style\""
        "widget \"*.fast-user-switch-menuitem\" style \"indicator-applet-menuitem-style\""
        "widget \"*.fast-user-switch-menubar\" style \"indicator-applet-menubar-style\"");
	//gtk_widget_set_name(GTK_WIDGET (applet), "indicator-applet-menubar");
	gtk_widget_set_name(GTK_WIDGET (applet), "fast-user-switch-applet");

	/* Build menubar */
	menubar = gtk_menu_bar_new();
	GTK_WIDGET_SET_FLAGS (menubar, GTK_WIDGET_FLAGS(menubar) | GTK_CAN_FOCUS);
	gtk_widget_set_name(GTK_WIDGET (menubar), "fast-user-switch-menubar");
	g_signal_connect(menubar, "button-press-event", G_CALLBACK(menubar_press), NULL);
	g_signal_connect_after(menubar, "expose-event", G_CALLBACK(menubar_on_expose), menubar);
	gtk_container_set_border_width(GTK_CONTAINER(menubar), 0);

	/* load 'em */
	if (g_file_test(INDICATOR_DIR, (G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))) {
		GDir * dir = g_dir_open(INDICATOR_DIR, 0, NULL);

		const gchar * name;
		while ((name = g_dir_read_name(dir)) != NULL) {
#ifdef INDICATOR_APPLET
			if (!g_strcmp0(name, "libsession.so")) {
				continue;
			}
			if (!g_strcmp0(name, "libme.so")) {
				continue;
			}
#endif
#ifdef INDICATOR_APPLET_SESSION
			if (g_strcmp0(name, "libsession.so") && g_strcmp0(name, "libme.so")) {
				continue;
			}
#endif
			if (load_module(name, menubar)) {
				indicators_loaded++;
			}
		}
		g_dir_close (dir);
	}

	if (indicators_loaded == 0) {
		/* A label to allow for click through */
		GtkWidget * item = gtk_label_new(_("No Indicators"));
		gtk_container_add(GTK_CONTAINER(applet), item);
		gtk_widget_show(item);
	} else {
		gtk_container_add(GTK_CONTAINER(applet), menubar);
		panel_applet_set_background_widget(applet, menubar);
		gtk_widget_show(menubar);
	}
  
	/* Background of applet */
	g_signal_connect(applet, "change-background",
			  G_CALLBACK(cw_panel_background_changed), menubar);
  
	gtk_widget_show(GTK_WIDGET(applet));

	return TRUE;
}

static void 
cw_panel_background_changed (PanelApplet               *applet,
                             PanelAppletBackgroundType  type,
                             GdkColor                  *colour,
                             GdkPixmap                 *pixmap,
                             GtkWidget                 *menubar)
{
	GtkRcStyle *rc_style;
	GtkStyle *style;

	/* reset style */
	gtk_widget_set_style(GTK_WIDGET (applet), NULL);
 	gtk_widget_set_style(menubar, NULL);
	rc_style = gtk_rc_style_new ();
	gtk_widget_modify_style(GTK_WIDGET (applet), rc_style);
	gtk_widget_modify_style(menubar, rc_style);
	gtk_rc_style_unref(rc_style);

	switch (type) 
	{
		case PANEL_NO_BACKGROUND:
			break;
		case PANEL_COLOR_BACKGROUND:
			gtk_widget_modify_bg(GTK_WIDGET (applet), GTK_STATE_NORMAL, colour);
			gtk_widget_modify_bg(menubar, GTK_STATE_NORMAL, colour);
			break;
    
		case PANEL_PIXMAP_BACKGROUND:
			style = gtk_style_copy(GTK_WIDGET (applet)->style);
			if (style->bg_pixmap[GTK_STATE_NORMAL])
				g_object_unref(style->bg_pixmap[GTK_STATE_NORMAL]);
			style->bg_pixmap[GTK_STATE_NORMAL] = g_object_ref (pixmap);
			gtk_widget_set_style(GTK_WIDGET (applet), style);
			gtk_widget_set_style(GTK_WIDGET (menubar), style);
			g_object_unref(style);
			break;
  }
}

