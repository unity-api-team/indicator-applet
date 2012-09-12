/*
A small wrapper utility to load indicators and put them as menu items
into the gnome-panel using it's applet interface.

Copyright 2009-2010 Canonical Ltd.

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
#include <string.h>
#include <config.h>
#include <glib/gi18n.h>
#include <panel-applet.h>
#include <gdk/gdkkeysyms.h>

#include "libindicator/indicator-object.h"
#include "tomboykeybinder.h"

static gchar * indicator_order[][2] = {
  {"libappmenu.so", NULL},
  {"libapplication.so", NULL},
  {"libapplication.so", "gst-keyboard-xkb"},
  {"libmessaging.so", NULL},
  {"libpower.so", NULL},
  {"libapplication.so", "bluetooth-manager"},
  {"libnetwork.so", NULL},
  {"libnetworkmenu.so", NULL},
  {"libapplication.so", "nm-applet"},
  {"libsoundmenu.so", NULL},
  {"libdatetime.so", NULL},
  {"libsession.so", NULL},
  {NULL, NULL}
};

static GtkPackDirection packdirection;
static PanelAppletOrient orient;

#define  MENU_DATA_BOX               "box"
#define  MENU_DATA_INDICATOR_OBJECT  "indicator-object"
#define  MENU_DATA_INDICATOR_ENTRY   "indicator-entry"
#define  MENU_DATA_IN_MENUITEM       "in-menuitem"
#define  MENU_DATA_MENUITEM_PRESSED  "menuitem-pressed"

#define  IO_DATA_NAME                "indicator-name"
#define  IO_DATA_ORDER_NUMBER        "indicator-order-number"
#define  IO_DATA_MENUITEM_LOOKUP     "indicator-menuitem-lookup"

static gboolean applet_fill_cb (PanelApplet * applet, const gchar * iid, gpointer data);

static void update_accessible_desc (IndicatorObjectEntry * entry, GtkWidget * menuitem);

/*************
 * main
 * ***********/

#ifdef INDICATOR_APPLET
PANEL_APPLET_OUT_PROCESS_FACTORY ("IndicatorAppletFactory",
               PANEL_TYPE_APPLET,
               applet_fill_cb, NULL);
#endif
#ifdef INDICATOR_APPLET_SESSION
PANEL_APPLET_OUT_PROCESS_FACTORY ("FastUserSwitchAppletFactory",
               PANEL_TYPE_APPLET,
               applet_fill_cb, NULL);
#endif
#ifdef INDICATOR_APPLET_COMPLETE
PANEL_APPLET_OUT_PROCESS_FACTORY ("IndicatorAppletCompleteFactory",
               PANEL_TYPE_APPLET,
               applet_fill_cb, NULL);
#endif
#ifdef INDICATOR_APPLET_APPMENU
PANEL_APPLET_OUT_PROCESS_FACTORY ("IndicatorAppletAppmenuFactory",
               PANEL_TYPE_APPLET,
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
#ifdef INDICATOR_APPLET_APPMENU
#define LOG_FILE_NAME  "indicator-applet-appmenu.log"
#endif
static FILE *log_file = NULL;

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
#ifdef INDICATOR_APPLET_APPMENU
gchar * hotkey_keycode = "<Super>F1";
#endif

/********************
 * Environment Names
 * *******************/
#ifdef INDICATOR_APPLET
#define INDICATOR_SPECIFIC_ENV  "indicator-applet-original"
#endif
#ifdef INDICATOR_APPLET_SESSION
#define INDICATOR_SPECIFIC_ENV  "indicator-applet-session"
#endif
#ifdef INDICATOR_APPLET_COMPLETE
#define INDICATOR_SPECIFIC_ENV  "indicator-applet-complete"
#endif
#ifdef INDICATOR_APPLET_APPMENU
#define INDICATOR_SPECIFIC_ENV  "indicator-applet-appmenu"
#endif

static const gchar * indicator_env[] = {
  "indicator-applet",
  INDICATOR_SPECIFIC_ENV,
  NULL
};

static gint
name2order (const gchar * name, const gchar * hint) {
  int i;

  for (i = 0; indicator_order[i][0] != NULL; i++) {
    if (g_strcmp0(name, indicator_order[i][0]) == 0 &&
        g_strcmp0(hint, indicator_order[i][1]) == 0) {
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
place_in_menu_cb (GtkWidget * widget, gpointer user_data)
{
  incoming_position_t * position = (incoming_position_t *)user_data;
  if (position->found) {
    /* We've already been placed, just finish the foreach */
    return;
  }

  IndicatorObject * io = INDICATOR_OBJECT(g_object_get_data(G_OBJECT(widget), MENU_DATA_INDICATOR_OBJECT));
  g_return_if_fail(INDICATOR_IS_OBJECT(io));

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

/* Position the entry */
static void
place_in_menu (GtkWidget *menubar, 
               GtkWidget *menuitem, 
               IndicatorObject *io, 
               IndicatorObjectEntry *entry)
{
  incoming_position_t position;

  /* Start with the default position for this indicator object */
  gint io_position = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(io), IO_DATA_ORDER_NUMBER));

  /* If name-hint is set, try to find the entry's position */
  if (entry->name_hint != NULL) {
    const gchar *name = (const gchar *)g_object_get_data(G_OBJECT(io), IO_DATA_NAME);
    gint entry_position = name2order(name, entry->name_hint);

    /* If we don't find the entry, fall back to the indicator object's position */
    if (entry_position > -1)
      io_position = entry_position;
  }

  position.objposition = io_position;
  position.entryposition = indicator_object_get_location(io, entry);
  position.menupos = 0;
  position.found = FALSE;

  gtk_container_foreach(GTK_CONTAINER(menubar), place_in_menu_cb, &position);

  gtk_menu_shell_insert(GTK_MENU_SHELL(menubar), menuitem, position.menupos);
}

static void
something_shown (GtkWidget * widget, gpointer user_data)
{
  GtkWidget * menuitem = GTK_WIDGET(user_data);
  gtk_widget_show(menuitem);
}

static void
something_hidden (GtkWidget * widget, gpointer user_data)
{
  GtkWidget * menuitem = GTK_WIDGET(user_data);
  gtk_widget_hide(menuitem);
}

static void
sensitive_cb (GObject * obj, GParamSpec * pspec, gpointer user_data)
{
  g_return_if_fail(GTK_IS_WIDGET(obj));
  g_return_if_fail(GTK_IS_WIDGET(user_data));

  gtk_widget_set_sensitive(GTK_WIDGET(user_data), gtk_widget_get_sensitive(GTK_WIDGET(obj)));
  return;
}

static void
entry_activated (GtkWidget * widget, gpointer user_data)
{
  g_return_if_fail(GTK_IS_WIDGET(widget));

  IndicatorObject *io = g_object_get_data (G_OBJECT (widget), MENU_DATA_INDICATOR_OBJECT);
  IndicatorObjectEntry *entry = g_object_get_data (G_OBJECT (widget), MENU_DATA_INDICATOR_ENTRY);

  g_return_if_fail(INDICATOR_IS_OBJECT(io));

  return indicator_object_entry_activate(io, entry, gtk_get_current_event_time());
}

static gboolean
entry_secondary_activated (GtkWidget * widget, GdkEvent * event, gpointer user_data)
{
  g_return_val_if_fail(GTK_IS_WIDGET(widget), FALSE);

  switch (event->type) {
    case GDK_ENTER_NOTIFY:
      g_object_set_data(G_OBJECT(widget), MENU_DATA_IN_MENUITEM, GINT_TO_POINTER(TRUE));
      break;

    case GDK_LEAVE_NOTIFY:
      g_object_set_data(G_OBJECT(widget), MENU_DATA_IN_MENUITEM, GINT_TO_POINTER(FALSE));
      g_object_set_data(G_OBJECT(widget), MENU_DATA_MENUITEM_PRESSED, GINT_TO_POINTER(FALSE));
      break;

    case GDK_BUTTON_PRESS:
      if (event->button.button == 2) {
        g_object_set_data(G_OBJECT(widget), MENU_DATA_MENUITEM_PRESSED, GINT_TO_POINTER(TRUE));
      }
      break;

    case GDK_BUTTON_RELEASE:
      if (event->button.button == 2) {
        gboolean in_menuitem = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), MENU_DATA_IN_MENUITEM));
        gboolean menuitem_pressed = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), MENU_DATA_MENUITEM_PRESSED));

        if (in_menuitem && menuitem_pressed) {
          g_object_set_data(G_OBJECT(widget), MENU_DATA_MENUITEM_PRESSED, GINT_TO_POINTER(FALSE));

          IndicatorObject *io = g_object_get_data(G_OBJECT(widget), MENU_DATA_INDICATOR_OBJECT);
          IndicatorObjectEntry *entry = g_object_get_data(G_OBJECT(widget), MENU_DATA_INDICATOR_ENTRY);

          g_return_val_if_fail(INDICATOR_IS_OBJECT(io), FALSE);

          g_signal_emit_by_name(io, INDICATOR_OBJECT_SIGNAL_SECONDARY_ACTIVATE, 
              entry, event->button.time);
        }
      }
      break;
  }

  return FALSE;
}

static gboolean
entry_scrolled (GtkWidget *menuitem, GdkEventScroll *event, gpointer data)
{
  g_return_val_if_fail(GTK_IS_WIDGET(menuitem), FALSE);

  IndicatorObject *io = g_object_get_data (G_OBJECT (menuitem), MENU_DATA_INDICATOR_OBJECT);
  IndicatorObjectEntry *entry = g_object_get_data (G_OBJECT (menuitem), MENU_DATA_INDICATOR_ENTRY);

  g_return_val_if_fail(INDICATOR_IS_OBJECT(io), FALSE);

  g_signal_emit_by_name (io, INDICATOR_OBJECT_SIGNAL_ENTRY_SCROLLED, entry, 1, event->direction);

  return FALSE;
}

static void
accessible_desc_update_cb (GtkWidget * widget, gpointer userdata)
{
  gpointer data = g_object_get_data(G_OBJECT(widget), MENU_DATA_INDICATOR_ENTRY);

  if (data != userdata) {
    return;
  }

  IndicatorObjectEntry * entry = (IndicatorObjectEntry *)data;
  update_accessible_desc(entry, widget);
}

static void
accessible_desc_update (IndicatorObject * io, IndicatorObjectEntry * entry, GtkWidget * menubar)
{
  gtk_container_foreach(GTK_CONTAINER(menubar), accessible_desc_update_cb, entry);
  return;
}

static GtkWidget*
create_menuitem (IndicatorObject * io, IndicatorObjectEntry * entry, GtkWidget * menubar)
{
  GtkWidget * box;
  GtkWidget * menuitem;

  menuitem = gtk_menu_item_new();
  box = (packdirection == GTK_PACK_DIRECTION_LTR)
      ? gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3)
      : gtk_box_new (GTK_ORIENTATION_VERTICAL, 3);

  gtk_widget_add_events(GTK_WIDGET(menuitem), GDK_SCROLL_MASK);

  g_object_set_data (G_OBJECT (menuitem), MENU_DATA_BOX, box);
  g_object_set_data(G_OBJECT(menuitem), MENU_DATA_INDICATOR_ENTRY,  entry);
  g_object_set_data(G_OBJECT(menuitem), MENU_DATA_INDICATOR_OBJECT, io);

  g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(entry_activated), NULL);
  g_signal_connect(G_OBJECT(menuitem), "button-press-event", G_CALLBACK(entry_secondary_activated), NULL);
  g_signal_connect(G_OBJECT(menuitem), "button-release-event", G_CALLBACK(entry_secondary_activated), NULL);
  g_signal_connect(G_OBJECT(menuitem), "enter-notify-event", G_CALLBACK(entry_secondary_activated), NULL);
  g_signal_connect(G_OBJECT(menuitem), "leave-notify-event", G_CALLBACK(entry_secondary_activated), NULL);
  g_signal_connect(G_OBJECT(menuitem), "scroll-event", G_CALLBACK(entry_scrolled), NULL);

  if (entry->image != NULL) {
    gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(entry->image), FALSE, FALSE, 1);
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
    gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(entry->label), FALSE, FALSE, 1);
  }
  gtk_container_add(GTK_CONTAINER(menuitem), box);
  gtk_widget_show(box);

  if (entry->menu != NULL) {
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), GTK_WIDGET(entry->menu));
  }

  place_in_menu(menubar, menuitem, io, entry);

  return menuitem;
}

static void
entry_added (IndicatorObject * io, IndicatorObjectEntry * entry, GtkWidget * menubar)
{
  const gchar * name;
  GtkWidget * menuitem;
  GHashTable * menuitem_lookup;
  gboolean something_visible;
  gboolean something_sensitive;

  name = g_object_get_data (G_OBJECT(io), IO_DATA_NAME);
  g_debug ("Signal: Entry Added from %s", name);

  /* if the menuitem doesn't already exist, create it now */
  menuitem_lookup = g_object_get_data (G_OBJECT(io), IO_DATA_MENUITEM_LOOKUP);
  g_return_if_fail (menuitem_lookup != NULL);
  menuitem = g_hash_table_lookup (menuitem_lookup, entry);
  if (menuitem == NULL) {
    menuitem = create_menuitem (io, entry, menubar);
    g_hash_table_insert (menuitem_lookup, entry, menuitem);
  }

  /* connect the callbacks */
  if (G_IS_OBJECT (entry->image)) {
    g_object_connect (entry->image,
                      "signal::show", G_CALLBACK(something_shown), menuitem,
                      "signal::hide", G_CALLBACK(something_hidden), menuitem,
                      "signal::notify::sensitive", G_CALLBACK(sensitive_cb), menuitem,
                      NULL);
  }
  if (G_IS_OBJECT (entry->label)) {
    g_object_connect (entry->label,
                      "signal::show", G_CALLBACK(something_shown), menuitem,
                      "signal::hide", G_CALLBACK(something_hidden), menuitem,
                      "signal::notify::sensitive", G_CALLBACK(sensitive_cb), menuitem,
                      NULL);
  }

  /* refresh based on visibility & sensitivity */
  something_visible = FALSE;
  something_sensitive = FALSE;
  if (entry->image != NULL) {
    GtkWidget * w = GTK_WIDGET (entry->image);
    something_visible |= gtk_widget_get_visible (w);
    something_sensitive |= gtk_widget_get_sensitive (w);
  }
  if (entry->label != NULL) {
    GtkWidget * w = GTK_WIDGET (entry->label);
    something_visible |= gtk_widget_get_visible (w);
    something_sensitive |= gtk_widget_get_sensitive (w);
  }
  if (something_visible) {
    if (entry->accessible_desc != NULL) {
      update_accessible_desc(entry, menuitem);
    }
    gtk_widget_show(menuitem);
  }
  gtk_widget_set_sensitive(menuitem, something_sensitive);

  return;
}

static void
entry_removed (IndicatorObject * io,
               IndicatorObjectEntry * entry,
               gpointer user_data)
{
  GtkWidget * menuitem;
  GHashTable * menuitem_lookup;

  g_debug("Signal: Entry Removed");

  menuitem_lookup = g_object_get_data (G_OBJECT(io), IO_DATA_MENUITEM_LOOKUP);
  g_return_if_fail (menuitem_lookup != NULL);
  menuitem = g_hash_table_lookup (menuitem_lookup, entry);
  g_return_if_fail (menuitem != NULL);

  /* disconnect the callbacks */
  if (G_IS_OBJECT (entry->label)) {
    g_object_disconnect (entry->label,
                         "signal::show", G_CALLBACK(something_shown), menuitem,
                         "signal::hide", G_CALLBACK(something_hidden), menuitem,
                         "signal::notify::sensitive", G_CALLBACK(sensitive_cb), menuitem,
                         NULL);
  }
  if (G_IS_OBJECT (entry->image)) {
    g_object_disconnect (entry->image,
                         "signal::show", G_CALLBACK(something_shown), menuitem,
                         "signal::hide", G_CALLBACK(something_hidden), menuitem,
                         "signal::notify::sensitive", G_CALLBACK(sensitive_cb), menuitem,
                         NULL);
  }

  gtk_widget_hide (menuitem);

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
entry_moved (IndicatorObject * io, IndicatorObjectEntry * entry,
             gint old G_GNUC_UNUSED, gint new G_GNUC_UNUSED, gpointer user_data)
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
  place_in_menu(menubar, mi, io, entry);
  g_object_unref(G_OBJECT(mi));

  return;
}

static void
menu_show (IndicatorObject * io, IndicatorObjectEntry * entry,
           guint32 timestamp, gpointer user_data)
{
  GtkWidget * menubar = GTK_WIDGET(user_data);

  if (entry == NULL) {
    /* Close any open menus instead of opening one */
    GList * l;
    GList * entries = indicator_object_get_entries(io);
    for (l = entries; l != NULL; l = g_list_next(entry)) {
      IndicatorObjectEntry * entrydata = l->data;
      gtk_menu_popdown(entrydata->menu);
    }
    g_list_free(entries);

    /* And tell the menubar to exit activation mode too */
    gtk_menu_shell_cancel(GTK_MENU_SHELL(menubar));
    return;
  }

  // TODO: do something sensible here
}

static void
update_accessible_desc(IndicatorObjectEntry * entry, GtkWidget * menuitem)
{
  /* FIXME: We need to deal with the use case where the contents of the
     label overrides what is found in the atk object's name, or at least
     orca speaks the label instead of the atk object name.
   */
  AtkObject * menuitem_obj = gtk_widget_get_accessible(menuitem);
  if (menuitem_obj == NULL) {
    /* Should there be an error printed here? */
    return;
  }

  if (entry->accessible_desc != NULL) {
    atk_object_set_name(menuitem_obj, entry->accessible_desc);
  } else {
    atk_object_set_name(menuitem_obj, "");
  }
  return;
}


static gboolean
load_module (const gchar * name, GtkWidget * menubar)
{
  GObject * o;

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

  /* Set the environment it's in */
  indicator_object_set_environment(io, (GStrv)indicator_env);

  /* Attach the 'name' to the object */
  o = G_OBJECT (io);
  g_object_set_data_full(o, IO_DATA_MENUITEM_LOOKUP, g_hash_table_new (g_direct_hash, g_direct_equal), (GDestroyNotify)g_hash_table_destroy);
  g_object_set_data_full(o, IO_DATA_NAME, g_strdup(name), g_free);
  g_object_set_data(o, IO_DATA_ORDER_NUMBER, GINT_TO_POINTER(name2order(name, NULL)));

  /* Connect to its signals */
  g_signal_connect(o, INDICATOR_OBJECT_SIGNAL_ENTRY_ADDED,   G_CALLBACK(entry_added),    menubar);
  g_signal_connect(o, INDICATOR_OBJECT_SIGNAL_ENTRY_REMOVED, G_CALLBACK(entry_removed),  menubar);
  g_signal_connect(o, INDICATOR_OBJECT_SIGNAL_ENTRY_MOVED,   G_CALLBACK(entry_moved),    menubar);
  g_signal_connect(o, INDICATOR_OBJECT_SIGNAL_MENU_SHOW,     G_CALLBACK(menu_show),      menubar);
  g_signal_connect(o, INDICATOR_OBJECT_SIGNAL_ACCESSIBLE_DESC_UPDATE, G_CALLBACK(accessible_desc_update), menubar);

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

  g_debug ("Hotkey: %s", keystring);

  /* Oh, wow, it's us! */
  GList * children = gtk_container_get_children(GTK_CONTAINER(data));
  if (children == NULL) {
    g_debug("Menubar has no children");
    return;
  }

  gtk_menu_shell_select_item(GTK_MENU_SHELL(data), GTK_WIDGET(g_list_last(children)->data));
  g_list_free(children);
  return;
}

static gboolean
menubar_press (GtkWidget * widget,
                    GdkEventButton *event,
                    gpointer data G_GNUC_UNUSED)
{
  if (event->button != 1) {
    g_signal_stop_emission_by_name(widget, "button-press-event");
  }

  return FALSE;
}

static void
about_cb (GtkAction *action G_GNUC_UNUSED,
          gpointer   data G_GNUC_UNUSED)
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
    "copyright", "Copyright \xc2\xa9 2009-2010 Canonical, Ltd.",
#ifdef INDICATOR_APPLET_SESSION
    "comments", _("A place to adjust your status, change users or exit your session."),
#else
#ifdef INDICATOR_APPLET_APPMENU
    "comments", _("An applet to hold your application menus."),
#endif
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
  GtkWidget *from = g_object_get_data(G_OBJECT(menuitem), MENU_DATA_BOX);
  GtkWidget *to = (packdirection == GTK_PACK_DIRECTION_LTR) ?
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0) : gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  g_object_set_data(G_OBJECT(from), "to", to);
  gtk_container_foreach(GTK_CONTAINER(from), (GtkCallback)swap_orient_cb,
      from);
  gtk_container_remove(GTK_CONTAINER(menuitem), from);
  gtk_container_add(GTK_CONTAINER(menuitem), to);
  g_object_set_data(G_OBJECT(menuitem), MENU_DATA_BOX, to);
  gtk_widget_show_all(menuitem);
  return TRUE;
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
    gtk_container_foreach(GTK_CONTAINER(menubar),
        (GtkCallback)reorient_box_cb, NULL);
  }
  orient = neworient;
  return FALSE;
}

#ifdef N_
#undef N_
#endif
#define N_(x) x

static void
log_to_file (const gchar * domain,
             GLogLevelFlags level,
             const gchar * message,
             gpointer data)
{
  if (log_file == NULL) {
    gchar *path;

    g_mkdir_with_parents(g_get_user_cache_dir(), 0755);
    path = g_build_filename(g_get_user_cache_dir(), LOG_FILE_NAME, NULL);

    log_file = fopen(path, "w");

    g_free(path);
  }

  if(log_file) {
    const gchar *prefix;

    switch(level & G_LOG_LEVEL_MASK) {
      case G_LOG_LEVEL_ERROR:
        prefix = "ERROR:";
        break;
      case G_LOG_LEVEL_CRITICAL:
        prefix = "CRITICAL:";
        break;
      case G_LOG_LEVEL_WARNING:
        prefix = "WARNING:";
        break;
      case G_LOG_LEVEL_MESSAGE:
        prefix = "MESSAGE:";
        break;
      case G_LOG_LEVEL_INFO:
        prefix = "INFO:";
        break;
      case G_LOG_LEVEL_DEBUG:
        prefix = "DEBUG:";
        break;
      default:
        prefix = "LOG:";
        break;
    }

    fprintf(log_file, "%s %s - %s\n", prefix, domain, message);
    fflush(log_file);
  }

  g_log_default_handler(domain, level, message, data);

  return;
}

static gboolean
applet_fill_cb (PanelApplet * applet, const gchar * iid G_GNUC_UNUSED,
                gpointer data G_GNUC_UNUSED)
{
  static const GtkActionEntry menu_actions[] = {
    {"About", GTK_STOCK_ABOUT, N_("_About"), NULL, NULL, G_CALLBACK(about_cb)}
  };
  static const gchar *menu_xml = "<menuitem name=\"About\" action=\"About\"/>";

  static gboolean first_time = FALSE;
  GtkWidget *menubar;
  gint indicators_loaded = 0;
  GtkActionGroup *action_group;

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
#ifdef INDICATOR_APPLET_APPMENU
    g_set_application_name(_("Indicator Applet Application Menu"));
#endif
    
    g_log_set_default_handler(log_to_file, NULL);

    tomboy_keybinder_init();
  }

  /* Set panel options */
  gtk_container_set_border_width(GTK_CONTAINER (applet), 0);
  panel_applet_set_flags(applet, PANEL_APPLET_EXPAND_MINOR);
  menubar = gtk_menu_bar_new();
  action_group = gtk_action_group_new ("Indicator Applet Actions");
  gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
  gtk_action_group_add_actions (action_group, menu_actions,
                                G_N_ELEMENTS (menu_actions),
                                menubar);
  panel_applet_setup_menu(applet, menu_xml, action_group);
  g_object_unref(action_group);
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
#ifdef INDICATOR_APPLET_APPMENU
  atk_object_set_name (gtk_widget_get_accessible (GTK_WIDGET (applet)),
                       "indicator-applet-appmenu");
#endif

  /* Init some theme/icon stuff */
  gtk_icon_theme_append_search_path(gtk_icon_theme_get_default(),
                                    INDICATOR_ICONS_DIR);
  g_debug("Icons directory: %s", INDICATOR_ICONS_DIR);

  gtk_widget_set_name(GTK_WIDGET (applet), "fast-user-switch-applet");

  /* Build menubar */
  orient = (panel_applet_get_orient(applet));
  packdirection = ((orient == PANEL_APPLET_ORIENT_UP) ||
      (orient == PANEL_APPLET_ORIENT_DOWN)) ? 
      GTK_PACK_DIRECTION_LTR : GTK_PACK_DIRECTION_TTB;
  gtk_menu_bar_set_pack_direction(GTK_MENU_BAR(menubar),
      packdirection);
  gtk_widget_set_can_focus (GTK_WIDGET (menubar), TRUE);
  gtk_widget_set_name(GTK_WIDGET (menubar), "fast-user-switch-menubar");
  g_signal_connect(menubar, "button-press-event", G_CALLBACK(menubar_press), NULL);
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
#ifdef INDICATOR_APPLET_APPMENU
      if (g_strcmp0(name, "libappmenu.so")) {
        continue;
      }
#else
      if (!g_strcmp0(name, "libappmenu.so")) {
        continue;
      }
#endif
#ifdef INDICATOR_APPLET
      if (!g_strcmp0(name, "libsession.so")) {
        continue;
      }
      if (!g_strcmp0(name, "libme.so")) {
        continue;
      }
      if (!g_strcmp0(name, "libdatetime.so")) {
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

  gtk_widget_show(GTK_WIDGET(applet));

  return TRUE;
}

