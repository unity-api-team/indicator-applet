/*
A small wrapper utility to load indicators and put them as menu items
into Avant Window Navigator using its applet interface.

Copyright 2009 Canonical Ltd.

Authors:
    Mark Lee <avant-wn@lazymalevolence.com>

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

#include <libawn/awn-applet.h>
#include "shared.c"

AwnApplet*
awn_applet_factory_initp (gchar* uid, gint orient, gint height)
{
  AwnApplet *applet;
  GtkWidget *menubar;

  applet = awn_applet_new (uid, orient, height);
  initialize_icon_settings (GTK_WIDGET (applet));
  menubar = build_menubar ();
  gtk_container_add (GTK_CONTAINER (applet), menubar);
  gtk_widget_show (menubar);
  load_indicators (menubar);
  return applet;
}
/* vim: set et ts=2 sts=2 sw=2 ai cindent : */
