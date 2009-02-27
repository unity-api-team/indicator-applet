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

#include <panel-applet.h>
#include "shared.c"

static gboolean     applet_fill_cb (PanelApplet * applet, const gchar * iid, gpointer data);


static void cw_panel_background_changed (PanelApplet               *applet,
                                         PanelAppletBackgroundType  type,
                        				         GdkColor                  *colour,
                        				         GdkPixmap                 *pixmap,
                                         GtkWidget                 *menubar);

/*************
 * main
 * ***********/

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_IndicatorApplet_Factory",
               PANEL_TYPE_APPLET,
               "indicator-applet", "0",
               applet_fill_cb, NULL);

static gboolean
applet_fill_cb (PanelApplet * applet, const gchar * iid, gpointer data)
{
	GtkWidget *menubar;
  
	/* Set panel options */
	gtk_container_set_border_width(GTK_CONTAINER (applet), 0);
	panel_applet_set_flags(applet, PANEL_APPLET_EXPAND_MINOR | PANEL_APPLET_HAS_HANDLE);
  
	/* Init some theme/icon stuff */
	initialize_icon_settings(GTK_WIDGET (applet));

	/* Build menubar */
	menubar = build_menubar();

	gtk_container_add(GTK_CONTAINER(applet), menubar);
	panel_applet_set_background_widget(applet, menubar);
	gtk_widget_show(menubar);

	/* load 'em */
	load_indicators(menubar);
  
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

