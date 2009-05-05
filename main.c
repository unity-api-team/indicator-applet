#include <glib.h>
#include <gio/gio.h>
#include <string.h>

#include <clutter/clutter.h>
#include <clutter-gtk/clutter-gtk.h>

#include <libindicator/indicator.h>

#define ICONS_DIR  (DATADIR G_DIR_SEPARATOR_S "indicator-applet" G_DIR_SEPARATOR_S "icons")
#define INDICATOR_DIR "/usr/lib/indicators/2/"


/* globals */
static ClutterActor *stage = NULL;
static ClutterActor *texture = NULL;

/* image */
static gboolean load_module (const gchar *name);

int 
main (int argc, char *argv[])
{
	GtkWidget *window, *embed;

	gtk_clutter_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_resize (GTK_WINDOW (window), 32, 32);
	gtk_widget_show (window);
	g_signal_connect (window, "destroy", G_CALLBACK (clutter_main_quit), NULL);

	embed = gtk_clutter_embed_new ();
	gtk_container_add (GTK_CONTAINER (window), embed);
	gtk_widget_show (embed);

	stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (embed));
	clutter_actor_set_size (stage, 32, 32);
	clutter_actor_show (stage);

	if (g_file_test(INDICATOR_DIR, (G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))) 
	{
		GDir * dir = g_dir_open(INDICATOR_DIR, 0, NULL);

		const gchar * name;
		while ((name = g_dir_read_name(dir)) != NULL) 
		{
			load_module(name);
		}
	}

	clutter_main ();

  return 0;
}

static void
update_texture (GObject *object)
{
	GtkImageType type;
	
	type = gtk_image_get_storage_type (GTK_IMAGE (object));

	if (type == GTK_IMAGE_PIXMAP)
	{
		GdkPixbuf *pixbuf;

		pixbuf = gtk_image_get_pixbuf (GTK_IMAGE (object));

		gtk_clutter_texture_set_from_pixbuf (CLUTTER_TEXTURE (texture), pixbuf);

		g_object_unref (pixbuf);
	}
	else if (type == GTK_IMAGE_STOCK)
	{	
		gchar *stock_id = NULL;

		g_object_get (object, "stock", &stock_id, NULL);

		gtk_clutter_texture_set_from_stock (CLUTTER_TEXTURE (texture),
																				GTK_WIDGET (object),
																			  stock_id,
																				24);
		g_free (stock_id);
	}
	else if (type == GTK_IMAGE_ICON_NAME)
	{
		gchar *icon_name = NULL;

		g_object_get (object, "icon-name", &icon_name, NULL);

		gtk_clutter_texture_set_from_icon_name (CLUTTER_TEXTURE (texture),
																						GTK_WIDGET (object),
																					  icon_name,
																						24);

		g_free (icon_name);
	}

	clutter_actor_queue_redraw (texture);
}

static void
on_button_press (ClutterActor *texture, ClutterButtonEvent *event)
{
	if (event->button == 1)
	{
		GtkMenu *menu;
		
		menu = GTK_MENU (g_object_get_data (G_OBJECT (texture), "menu"));

		gtk_menu_popup (menu, NULL, NULL, NULL, NULL, 1, event->time);
	}
}

static gboolean
load_module (const gchar *name)
{
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

	get_version_t lget_version = NULL;
	g_return_val_if_fail(g_module_symbol(module, INDICATOR_GET_VERSION_S, (gpointer *)(&lget_version)), FALSE);
	if (!INDICATOR_VERSION_CHECK(lget_version())) {
		g_warning("Indicator using API version '%s' we're expecting '%s'", lget_version(), INDICATOR_VERSION);
		return FALSE;
	}

	get_label_t lget_label = NULL;
	g_return_val_if_fail(g_module_symbol(module, INDICATOR_GET_LABEL_S, (gpointer *)(&lget_label)), FALSE);
	g_return_val_if_fail(lget_label != NULL, FALSE);
	GtkLabel * label = lget_label();

	get_icon_t lget_icon = NULL;
	g_return_val_if_fail(g_module_symbol(module, INDICATOR_GET_ICON_S, (gpointer *)(&lget_icon)), FALSE);
	g_return_val_if_fail(lget_icon != NULL, FALSE);
	GtkImage * icon = lget_icon();

	get_menu_t lget_menu = NULL;
	g_return_val_if_fail(g_module_symbol(module, INDICATOR_GET_MENU_S, (gpointer *)(&lget_menu)), FALSE);
	g_return_val_if_fail(lget_menu != NULL, FALSE);
	GtkMenu * lmenu = lget_menu();

	if (label == NULL && icon == NULL) {
		/* This is the case where there is nothing to display,
		   kinda odd that we'd have a module with nothing. */
		g_warning("No label or icon.  Odd.");
		return FALSE;
	}
	
	texture = clutter_texture_new ();
	clutter_actor_set_size (texture, 24, 24);
	clutter_actor_set_position (texture, 0, 0);
	clutter_container_add_actor (CLUTTER_CONTAINER (stage), texture);
	clutter_actor_set_reactive (texture, TRUE);
	clutter_actor_show (texture);

	g_object_set_data (G_OBJECT (texture), "menu", lmenu);

	g_signal_connect (texture, "button-press-event",
										G_CALLBACK (on_button_press), NULL);
	g_signal_connect (icon, "notify::pixbuf", 
									  G_CALLBACK (update_texture), NULL);
	g_signal_connect (icon, "notify::icon-name", 
							  		 G_CALLBACK (update_texture), NULL);

	update_texture (G_OBJECT (icon));
	
	g_debug ("%p %p %p", label, icon, lmenu);

	return TRUE;
}
