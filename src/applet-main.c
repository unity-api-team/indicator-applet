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

#include <stdlib.h>
#include <config.h>
#include <panel-applet.h>
#include <gdk/gdkkeysyms.h>

#include "libindicator/indicator-object.h"

static gchar * indicator_order[] = {
	"libapplication.so",
	"libsoundmenu.so",
	"libmessaging.so",
	"libdatetime.so",
	"libme.so",
	"libsession.so",
	NULL
};

static enum orienters {
	HORIZ_TO_VERT = 1,
	VERT_TO_HORIZ
} box_reorient;

static GtkPackDirection packdirection;
static PanelAppletOrient orient;

#define  MENU_DATA_INDICATOR_OBJECT  "indicator-object"
#define  MENU_DATA_INDICATOR_ENTRY   "indicator-entry"

#define  IO_DATA_ORDER_NUMBER        "indicator-order-number"

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
 * log files
 * ***********/
#ifdef INDICATOR_APPLET
#define LOG_FILE_NAME  "indicator-applet.log"
#endif
#ifdef INDICATOR_APPLET_SESSION
#define LOG_FILE_NAME  "indicator-applet-session.log"
#endif
#ifdef INDICATOR_APPLET_COMPLETE
#define LOG_FILE_NAME  "indicator-applet-complete.log"
#endif
GOutputStream * log_file = NULL;

/*****************
 * Hotkey support 
 * **************/
#ifdef INDICATOR_APPLET
gchar * hotkey_keycode = "<Super>M";
#endif
#ifdef INDICATOR_APPLET_SESSION
gchar * hotkey_keycode = "<Super>S";
#endif
#ifdef INDICATOR_APPLET_COMPLETE
gchar * hotkey_keycode = "<Super>S";
#endif

/*************
 * init function
 * ***********/

static gint
name2order (const gchar * name) {
	int i;

	for (i = 0; indicator_order[i] != NULL; i++) {
		if (g_strcmp0(name, indicator_order[i]) == 0) {
			return i;
		}
	}

	return -1;
}

typedef struct _incoming_position_t incoming_position_t;
struct _incoming_position_t {
	gint objposition;
	gint entryposition;
	gint menupos;
	gboolean found;
};

/* This function helps by determining where in the menu list
   this new entry should be placed.  It compares the objects
   that they're on, and then the individual entries.  Each
   is progressively more expensive. */
static void
place_in_menu (GtkWidget * widget, gpointer user_data)
{
	incoming_position_t * position = (incoming_position_t *)user_data;
	if (position->found) {
		/* We've already been placed, just finish the foreach */
		return;
	}

	IndicatorObject * io = INDICATOR_OBJECT(g_object_get_data(G_OBJECT(widget), MENU_DATA_INDICATOR_OBJECT));
	g_assert(io != NULL);

	gint objposition = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(io), IO_DATA_ORDER_NUMBER));
	/* We've already passed it, well, then this is where
	   we should be be.  Stop! */
	if (objposition > position->objposition) {
		position->found = TRUE;
		return;
	}

	/* The objects don't match yet, keep looking */
	if (objposition < position->objposition) {
		position->menupos++;
		return;
	}

	/* The objects are the same, let's start looking at entries. */
	IndicatorObjectEntry * entry = (IndicatorObjectEntry *)g_object_get_data(G_OBJECT(widget), MENU_DATA_INDICATOR_ENTRY);
	gint entryposition = indicator_object_get_location(io, entry);

	if (entryposition > position->entryposition) {
		position->found = TRUE;
		return;
	}

	if (entryposition < position->entryposition) {
		position->menupos++;
		return;
	}

	/* We've got the same object and the same entry.  Well,
	   let's just put it right here then. */
	position->found = TRUE;
	return;
}

static void
entry_added (IndicatorObject * io, IndicatorObjectEntry * entry, GtkWidget * menubar)
{
	g_debug("Signal: Entry Added");

	GtkWidget * menuitem = gtk_menu_item_new();
	GtkWidget * box = (packdirection == GTK_PACK_DIRECTION_LTR) ?
			gtk_hbox_new(FALSE, 3) : gtk_vbox_new(FALSE, 3);

	g_object_set_data (G_OBJECT (menuitem), "indicator", io);
	g_object_set_data (G_OBJECT (menuitem), "box", box);

	if (entry->image != NULL) {
		gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(entry->image), FALSE, FALSE, 0);
	}
	if (entry->label != NULL) {
		switch(packdirection) {
			case GTK_PACK_DIRECTION_LTR:
				gtk_label_set_angle(GTK_LABEL(entry->label), 0.0);
				break;
			case GTK_PACK_DIRECTION_TTB:
				gtk_label_set_angle(GTK_LABEL(entry->label),
						(orient == PANEL_APPLET_ORIENT_LEFT) ? 
						270.0 : 90.0);
				break;
			default:
				break;
		}		
		gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(entry->label), FALSE, FALSE, 0);
	}
	gtk_container_add(GTK_CONTAINER(menuitem), box);
	gtk_widget_show(box);

	if (entry->menu != NULL) {
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), GTK_WIDGET(entry->menu));
	}

	incoming_position_t position;
	position.objposition = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(io), IO_DATA_ORDER_NUMBER));
	position.entryposition = indicator_object_get_location(io, entry);
	position.menupos = 0;
	position.found = FALSE;

	gtk_container_foreach(GTK_CONTAINER(menubar), place_in_menu, &position);

	gtk_menu_shell_insert(GTK_MENU_SHELL(menubar), menuitem, position.menupos);
	gtk_widget_show(menuitem);

	g_object_set_data(G_OBJECT(menuitem), MENU_DATA_INDICATOR_ENTRY,  entry);
	g_object_set_data(G_OBJECT(menuitem), MENU_DATA_INDICATOR_OBJECT, io);

	return;
}

static void
entry_removed_cb (GtkWidget * widget, gpointer userdata)
{
	gpointer data = g_object_get_data(G_OBJECT(widget), MENU_DATA_INDICATOR_ENTRY);

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

static void
entry_moved_find_cb (GtkWidget * widget, gpointer userdata)
{
	gpointer * array = (gpointer *)userdata;
	if (array[1] != NULL) {
		return;
	}

	gpointer data = g_object_get_data(G_OBJECT(widget), MENU_DATA_INDICATOR_ENTRY);

	if (data != array[0]) {
		return;
	}

	array[1] = widget;
	return;
}

/* Gets called when an entry for an object was moved. */
static void
entry_moved (IndicatorObject * io, IndicatorObjectEntry * entry, gint old, gint new, gpointer user_data)
{
	GtkWidget * menubar = GTK_WIDGET(user_data);

	gpointer array[2];
	array[0] = entry;
	array[1] = NULL;

	gtk_container_foreach(GTK_CONTAINER(menubar), entry_moved_find_cb, array);
	if (array[1] == NULL) {
		g_warning("Moving an entry that isn't in our menus.");
		return;
	}

	GtkWidget * mi = GTK_WIDGET(array[1]);
	g_object_ref(G_OBJECT(mi));
	gtk_container_remove(GTK_CONTAINER(menubar), mi);

	incoming_position_t position;
	position.objposition = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(io), IO_DATA_ORDER_NUMBER));
	position.entryposition = indicator_object_get_location(io, entry);
	position.menupos = 0;
	position.found = FALSE;

	gtk_container_foreach(GTK_CONTAINER(menubar), place_in_menu, &position);

	gtk_menu_shell_insert(GTK_MENU_SHELL(menubar), mi, position.menupos);
	g_object_unref(G_OBJECT(mi));

	return;
}

static gboolean
load_module (const gchar * name, GtkWidget * menubar)
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

	/* Attach the 'name' to the object */
	g_object_set_data(G_OBJECT(io), IO_DATA_ORDER_NUMBER, GINT_TO_POINTER(name2order(name)));

	/* Connect to its signals */
	g_signal_connect(G_OBJECT(io), INDICATOR_OBJECT_SIGNAL_ENTRY_ADDED,   G_CALLBACK(entry_added),    menubar);
	g_signal_connect(G_OBJECT(io), INDICATOR_OBJECT_SIGNAL_ENTRY_REMOVED, G_CALLBACK(entry_removed),  menubar);
	g_signal_connect(G_OBJECT(io), INDICATOR_OBJECT_SIGNAL_ENTRY_MOVED,   G_CALLBACK(entry_moved),    menubar);

	/* Work on the entries */
	GList * entries = indicator_object_get_entries(io);
	GList * entry = NULL;

	for (entry = entries; entry != NULL; entry = g_list_next(entry)) {
		IndicatorObjectEntry * entrydata = (IndicatorObjectEntry *)entry->data;
		entry_added(io, entrydata, menubar);
	}

	g_list_free(entries);

	return TRUE;
}

static void
hotkey_filter (char * keystring, gpointer data)
{
	g_return_if_fail(GTK_IS_MENU_SHELL(data));

	/* Oh, wow, it's us! */
	GList * children = gtk_container_get_children(GTK_CONTAINER(data));
	if (children == NULL) {
		g_debug("Menubar has no children");
		return;
	}

	if (!GTK_MENU_SHELL(data)->active) {
		gtk_grab_add (GTK_WIDGET(data));
		GTK_MENU_SHELL(data)->have_grab = TRUE;
		GTK_MENU_SHELL(data)->active = TRUE;
	}

	gtk_menu_shell_select_item(GTK_MENU_SHELL(data), GTK_WIDGET(g_list_last(children)->data));
	g_list_free(children);
	return;
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
menubar_scroll (GtkWidget      *widget,
                GdkEventScroll *event,
                gpointer        data)
{
  GtkWidget *menuitem;
  GtkWidget *parent;

  menuitem = gtk_get_event_widget ((GdkEvent *)event);

  IndicatorObject *io = g_object_get_data (G_OBJECT (menuitem), "indicator");
  g_signal_emit_by_name (io, "scroll", 1, event->direction);
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

static gboolean
swap_orient_cb (GtkWidget *item, gpointer data)
{
	GtkWidget *from = (GtkWidget *) data;
	GtkWidget *to = (GtkWidget *) g_object_get_data(G_OBJECT(from), "to");
	g_object_ref(G_OBJECT(item));
	gtk_container_remove(GTK_CONTAINER(from), item);
	if (GTK_IS_LABEL(item)) {
			switch(packdirection) {
			case GTK_PACK_DIRECTION_LTR:
				gtk_label_set_angle(GTK_LABEL(item), 0.0);
				break;
			case GTK_PACK_DIRECTION_TTB:
				gtk_label_set_angle(GTK_LABEL(item),
						(orient == PANEL_APPLET_ORIENT_LEFT) ? 
						270.0 : 90.0);
				break;
			default:
				break;
		}		
	}
	gtk_box_pack_start(GTK_BOX(to), item, FALSE, FALSE, 0);
	return TRUE;
}

static gboolean
reorient_box_cb (GtkWidget *menuitem, gpointer data)
{
	int reorient = GPOINTER_TO_INT(data);
	GtkWidget *from = g_object_get_data(G_OBJECT(menuitem), "box");
	GtkWidget *to = (reorient == HORIZ_TO_VERT) ? gtk_vbox_new(FALSE, 0) :
			gtk_hbox_new(FALSE, 0);
	g_object_set_data(G_OBJECT(from), "to", to);
	gtk_container_foreach(GTK_CONTAINER(from), (GtkCallback)swap_orient_cb,
			from);
	gtk_container_remove(GTK_CONTAINER(menuitem), from);
	gtk_container_add(GTK_CONTAINER(menuitem), to);
	g_object_set_data(G_OBJECT(menuitem), "box", to);
	gtk_widget_show_all(menuitem);
	return TRUE;
}

static void 
reorient_boxes (GtkWidget *menubar, int reorient)
{
	gtk_container_foreach(GTK_CONTAINER(menubar), (GtkCallback)reorient_box_cb,
			GINT_TO_POINTER(reorient));
}

static gboolean
panelapplet_reorient_cb (GtkWidget *applet, PanelAppletOrient neworient,
		gpointer data)
{
	GtkWidget *menubar = (GtkWidget *)data;
	if ((((neworient == PANEL_APPLET_ORIENT_UP) || 
			(neworient == PANEL_APPLET_ORIENT_DOWN)) && 
			((orient == PANEL_APPLET_ORIENT_LEFT) || 
			(orient == PANEL_APPLET_ORIENT_RIGHT))) || 
			(((neworient == PANEL_APPLET_ORIENT_LEFT) || 
			(neworient == PANEL_APPLET_ORIENT_RIGHT)) && 
			((orient == PANEL_APPLET_ORIENT_UP) ||
			(orient == PANEL_APPLET_ORIENT_DOWN)))) {
		packdirection = (packdirection == GTK_PACK_DIRECTION_LTR) ?
				GTK_PACK_DIRECTION_TTB : GTK_PACK_DIRECTION_LTR;
		gtk_menu_bar_set_pack_direction(GTK_MENU_BAR(menubar),
				packdirection);
		orient = neworient;
		reorient_boxes(menubar, 
				(packdirection == GTK_PACK_DIRECTION_LTR) ?
				VERT_TO_HORIZ : HORIZ_TO_VERT);
	}
	orient = neworient;
	return FALSE;
}

#ifdef N_
#undef N_
#endif
#define N_(x) x

static void
log_to_file_cb (GObject * source_obj, GAsyncResult * result, gpointer user_data)
{
	g_free(user_data);
	return;
}

static void
log_to_file (const gchar * domain, GLogLevelFlags level, const gchar * message, gpointer data)
{
	if (log_file == NULL) {
		GError * error = NULL;
		gchar * filename = g_build_filename(g_get_user_cache_dir(), LOG_FILE_NAME, NULL);
		GFile * file = g_file_new_for_path(filename);
		g_free(filename);

		if (!g_file_test(g_get_user_cache_dir(), G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR)) {
			GFile * cachedir = g_file_new_for_path(g_get_user_cache_dir());
			g_file_make_directory_with_parents(cachedir, NULL, &error);

			if (error != NULL) {
				g_error("Unable to make directory '%s' for log file: %s", g_get_user_cache_dir(), error->message);
				return;
			}
		}

		g_file_delete(file, NULL, NULL);

		GFileIOStream * io = g_file_create_readwrite(file,
		                          G_FILE_CREATE_REPLACE_DESTINATION, /* flags */
		                          NULL, /* cancelable */
		                          &error); /* error */
		if (error != NULL) {
			g_error("Unable to replace file: %s", error->message);
			return;
		}

		log_file = g_io_stream_get_output_stream(G_IO_STREAM(io));
	}

	gchar * outputstring = g_strdup_printf("%s\n", message);
	g_output_stream_write_async(log_file,
	                            outputstring, /* data */
	                            strlen(outputstring), /* length */
	                            G_PRIORITY_LOW, /* priority */
	                            NULL, /* cancelable */
	                            log_to_file_cb, /* callback */
	                            outputstring); /* data */

	return;
}

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

	static gboolean first_time = FALSE;
	gint i;
	gint indicators_loaded = 0;
	GtkWidget *menubar;

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
		
		g_log_set_default_handler(log_to_file, NULL);

		tomboy_keybinder_init();
	}

	/* Set panel options */
	gtk_container_set_border_width(GTK_CONTAINER (applet), 0);
	panel_applet_set_flags(applet, PANEL_APPLET_EXPAND_MINOR);
	menubar = gtk_menu_bar_new();
	panel_applet_setup_menu(applet, menu_xml, menu_verbs, menubar);
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
	orient = (panel_applet_get_orient(applet));
	packdirection = ((orient == PANEL_APPLET_ORIENT_UP) ||
			(orient == PANEL_APPLET_ORIENT_DOWN)) ? 
			GTK_PACK_DIRECTION_LTR : GTK_PACK_DIRECTION_TTB;
	gtk_menu_bar_set_pack_direction(GTK_MENU_BAR(menubar),
			packdirection);
	GTK_WIDGET_SET_FLAGS (menubar, GTK_WIDGET_FLAGS(menubar) | GTK_CAN_FOCUS);
	gtk_widget_set_name(GTK_WIDGET (menubar), "fast-user-switch-menubar");
	g_signal_connect(menubar, "button-press-event", G_CALLBACK(menubar_press), NULL);
        g_signal_connect(menubar, "scroll-event", G_CALLBACK (menubar_scroll), NULL);
	g_signal_connect_after(menubar, "expose-event", G_CALLBACK(menubar_on_expose), menubar);
	g_signal_connect(applet, "change-orient", 
			G_CALLBACK(panelapplet_reorient_cb), menubar);
	gtk_container_set_border_width(GTK_CONTAINER(menubar), 0);

	/* Add in filter func */
	tomboy_keybinder_bind(hotkey_keycode, hotkey_filter, menubar);

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

