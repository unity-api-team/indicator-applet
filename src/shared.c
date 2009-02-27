/*
Shared code for the indicator GNOME Panel/Avant Window Navigator applets.

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

#include <string.h>
#include <gtk/gtk.h>

#define SYMBOL_NAME  "get_menu_item"
#define ICONS_DIR  (DATADIR G_DIR_SEPARATOR_S "indicator-applet" G_DIR_SEPARATOR_S "icons")

/*************
 * init function
 * ***********/
static gboolean
load_module (const gchar * name, GtkWidget * menu)
{
	g_debug("Looking at Module: %s", name);
	g_return_val_if_fail(name != NULL, FALSE);

	guint suffix_len = strlen(G_MODULE_SUFFIX);
	guint name_len = strlen(name);

	g_return_val_if_fail(name_len > suffix_len, FALSE);

	int i;
	for (i = 0; i < suffix_len; i++) {
		if (name[(name_len - suffix_len) + i] != (G_MODULE_SUFFIX)[i])
			return FALSE;
	}
	g_debug("Loading Module: %s", name);

	gchar * fullpath = g_build_filename(INDICATOR_DIR, name, NULL);
	GModule * module = g_module_open(fullpath,
                                     G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL);
	g_free(fullpath);
	g_return_val_if_fail(module != NULL, FALSE);

	GtkWidget * (*make_item)(void);
	g_return_val_if_fail(g_module_symbol(module, SYMBOL_NAME, (gpointer *)(&make_item)), FALSE);
	g_return_val_if_fail(make_item != NULL, FALSE);

	GtkWidget * menuitem = make_item();
	g_return_val_if_fail(GTK_MENU_ITEM(menuitem), FALSE);

	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	return TRUE;
}

static void
load_indicators (GtkWidget * menubar)
{
	gint indicators_loaded = 0;

	if (g_file_test(INDICATOR_DIR, (G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))) {
		GDir * dir = g_dir_open(INDICATOR_DIR, 0, NULL);

		const gchar * name;
		while ((name = g_dir_read_name(dir)) != NULL) {
			if (load_module(name, menubar))
				indicators_loaded++;
		}
	}

	if (indicators_loaded == 0) {
		GtkWidget * item = gtk_menu_item_new_with_label("No Indicators");
		gtk_widget_set_sensitive(item, FALSE);
		gtk_menu_shell_append(GTK_MENU_SHELL(menubar), item);
		gtk_widget_show(item);
	}
}

/**
 * Initializes some icon theme stuff.
 */
static void
initialize_icon_settings (GtkWidget * applet)
{
	gtk_icon_theme_append_search_path(gtk_icon_theme_get_default(),
	                                  ICONS_DIR);
	g_debug("Icons directory: %s", ICONS_DIR);
	gtk_rc_parse_string (
	    "style \"indicator-applet-style\"\n"
        "{\n"
        "    GtkMenuBar::shadow-type = none\n"
        "    GtkMenuBar::internal-padding = 0\n"
        "    GtkWidget::focus-line-width = 0\n"
        "    GtkWidget::focus-padding = 0\n"
        "}\n"
        "widget \"*.indicator-applet-menubar\" style \"indicator-applet-style\"");
	gtk_widget_set_name(applet, "indicator-applet-menubar");
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

/**
 * Build menubar.
 */
static GtkWidget*
build_menubar ()
{
	GtkWidget *menubar = gtk_menu_bar_new();
	GTK_WIDGET_SET_FLAGS (menubar, GTK_WIDGET_FLAGS(menubar) | GTK_CAN_FOCUS);
	g_signal_connect_after(menubar, "expose-event", G_CALLBACK(menubar_on_expose), menubar);
	gtk_container_set_border_width(GTK_CONTAINER(menubar), 0);
	return menubar;
}
