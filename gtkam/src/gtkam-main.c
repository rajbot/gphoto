/* gtkam-main.c
 *
 * Copyright (C) 2001 Lutz M�ller <urc8@rz.uni-karlsruhe.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details. 
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include "gtkam-main.h"

#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

#include <stdio.h>
#include <string.h>

#include <gdk/gdkkeysyms.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkprogressbar.h>
#include <gtk/gtkstatusbar.h>
#include <gtk/gtktoolbar.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkframe.h>
#include <gtk/gtkhpaned.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmenubar.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkfilesel.h>

#include <gphoto2/gphoto2-camera.h>

#include "gtkam-cancel.h"
#include "gtkam-chooser.h"
#include "gtkam-close.h"
#include "gtkam-config.h"
#include "gtkam-context.h"
#include "gtkam-debug.h"
#include "gtkam-delete.h"
#include "gtkam-error.h"
#include "gtkam-list.h"
#include "gtkam-preview.h"
#include "gtkam-status.h"
#include "gtkam-tree.h"
#include "gtkam-tree-item.h"
#include "gtkam-tree-item-cam.h"

#include "support.h"

struct _GtkamMainPrivate
{
	Camera *camera;

	GtkamTree *tree;
	GtkamList *list;

	GtkToggleButton *toggle_preview;

	GtkWidget *item_delete, *item_delete_all;
	GtkWidget *item_save, *menu_delete;
	GtkWidget *select_all, *select_none, *select_inverse;

	GtkWidget *status;

	GtkWidget *vbox;

	gboolean multi;
};

#define PARENT_TYPE GTK_TYPE_WINDOW
static GtkWindowClass *parent_class;

static void
gtkam_main_destroy (GtkObject *object)
{
	GtkamMain *m = GTKAM_MAIN (object);

	if (m->priv->camera) {
		gp_camera_unref (m->priv->camera);
		m->priv->camera = NULL;
	}

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gtkam_main_finalize (GtkObject *object)
{
	GtkamMain *m = GTKAM_MAIN (object);

	g_free (m->priv);

	GTK_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gtkam_main_class_init (GtkamMainClass *klass)
{
	GtkObjectClass *object_class;

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy  = gtkam_main_destroy;
	object_class->finalize = gtkam_main_finalize;

	parent_class = gtk_type_class (PARENT_TYPE);
}

static void
gtkam_main_init (GtkamMain *main)
{
	main->priv = g_new0 (GtkamMainPrivate, 1);
}

GtkType
gtkam_main_get_type (void)
{
	static GtkType main_type = 0;

	if (!main_type) {
		static const GtkTypeInfo main_info = {
			"GtkamMain",
			sizeof (GtkamMain),
			sizeof (GtkamMainClass),
			(GtkClassInitFunc)  gtkam_main_class_init,
			(GtkObjectInitFunc) gtkam_main_init,
			NULL, NULL, NULL};
		main_type = gtk_type_unique (PARENT_TYPE, &main_info);
	}

	return (main_type);
}

static void
on_thumbnails_toggled (GtkToggleButton *toggle, GtkamMain *m)
{
	gtkam_list_set_thumbnails (m->priv->list, toggle->active);
}

static void
on_save_selected_photos_activate (GtkMenuItem *item, GtkamMain *m)
{
	gtkam_list_save_selected (m->priv->list);
}

static void
on_save_selected_photos_clicked (GtkMenuItem *item, GtkamMain *m)
{
	gtkam_list_save_selected (m->priv->list);
}

static void
on_exit_activate (GtkMenuItem *item, GtkamMain *m)
{
	gtk_object_destroy (GTK_OBJECT (m));
}

static void
on_file_deleted (GtkamDelete *delete, Camera *camera, gboolean multi,
		 const gchar *folder, const gchar *name, GtkamMain *m)
{
	gtkam_list_update_folder (m->priv->list, camera, multi, folder);
}

static void
delete_selected (GtkamMain *m)
{
	GtkIconListItem *item;
	GtkamList *list = m->priv->list;
	guint i;
	GtkWidget *delete;

	if (!g_list_length (GTK_ICON_LIST (list)->selection))
		return;

	delete = gtkam_delete_new (m->priv->status);
	gtk_window_set_transient_for (GTK_WINDOW (delete), GTK_WINDOW (m));
	gtk_signal_connect (GTK_OBJECT (delete), "file_deleted",
			    GTK_SIGNAL_FUNC (on_file_deleted), m);
	for (i = 0; i < g_list_length (GTK_ICON_LIST (list)->selection); i++) {
		item = g_list_nth_data (GTK_ICON_LIST (list)->selection, i);
		gtkam_delete_add (GTKAM_DELETE (delete),
			gtk_object_get_data (GTK_OBJECT (item->entry),
				"camera"),
			GPOINTER_TO_INT (
				gtk_object_get_data (GTK_OBJECT (item->entry),
					"multi")),
			gtk_object_get_data (GTK_OBJECT (item->entry),
				"folder"),
			item->label);
	}
	gtk_widget_show (delete);
}

static void
on_delete_selected_photos_activate (GtkMenuItem *item, GtkamMain *m)
{
	delete_selected (m);
}

static void
on_all_deleted (GtkamDelete *delete, Camera *camera, gboolean multi,
		const gchar *folder, GtkamMain *m)
{
	g_return_if_fail (GTKAM_IS_MAIN (m));

	gtkam_list_update_folder (m->priv->list, camera, multi, folder);
}

static void
on_delete_selected_photos_clicked (GtkButton *button, GtkamMain *m)
{
	delete_selected (m);
}

static void
on_delete_all_photos_activate (GtkMenuItem *menu_item, GtkamMain *m)
{
	GtkWidget *delete;
	GtkamTreeItem *item;
	GList *selection;
	gint i;

	selection = GTK_TREE (m->priv->tree)->selection;
	if (!g_list_length (selection))
		return;

	delete = gtkam_delete_new (m->priv->status);
	for (i = 0; i < g_list_length (selection); i++) {
		item = g_list_nth_data (selection, i);
		gtkam_delete_add (GTKAM_DELETE (delete),
			gtkam_tree_item_get_camera (item),
			gtkam_tree_item_get_multi (item),
			gtkam_tree_item_get_folder (item), NULL);
	}
	gtk_signal_connect (GTK_OBJECT (delete), "file_deleted",
			    GTK_SIGNAL_FUNC (on_file_deleted), m);
	gtk_signal_connect (GTK_OBJECT (delete), "all_deleted",
			    GTK_SIGNAL_FUNC (on_all_deleted), m);

	gtk_window_set_transient_for (GTK_WINDOW (delete), GTK_WINDOW (m));
	gtk_widget_show (delete);
}

static void
on_select_all_activate (GtkMenuItem *item, GtkamMain *m)
{
	GtkIconList *ilist = GTK_ICON_LIST (m->priv->list);
	guint i;

	for (i = 0; i < g_list_length (ilist->icons); i++)
		gtk_icon_list_select_icon (ilist,
				g_list_nth_data (ilist->icons, i));
}

static void
on_select_none_activate (GtkMenuItem *item, GtkamMain *m)
{
	gtk_icon_list_unselect_all (GTK_ICON_LIST (m->priv->list));
}

static void
on_select_inverse_activate (GtkMenuItem *menu_item, GtkamMain *m)
{
	GtkIconList *ilist = GTK_ICON_LIST (m->priv->list);
	GtkIconListItem *item;
	guint i;

	for (i = 0; i < g_list_length (ilist->icons); i++) {
		item = g_list_nth_data (ilist->icons, i);
		if (item->state == GTK_STATE_SELECTED)
			gtk_icon_list_unselect_icon (ilist, item);
		else
			gtk_icon_list_select_icon (ilist, item);
	}
}

static void
on_camera_selected (GtkamChooser *chooser, Camera *camera,
		    gboolean multi, GtkamMain *m)
{
	GtkWidget *item;

	item = gtkam_tree_item_cam_new ();
	gtk_widget_show (item);
	gtk_tree_append (GTK_TREE (m->priv->tree), item);
	gtkam_tree_item_set_camera (GTKAM_TREE_ITEM (item), camera);
	gtkam_tree_item_set_multi (GTKAM_TREE_ITEM (item), multi);
	gtkam_tree_save (GTKAM_TREE (m->priv->tree));

	gtkam_main_set_camera (m, camera, multi);
}

static void
on_add_camera_activate (GtkMenuItem *item, GtkamMain *m)
{
	GtkWidget *dialog;

	dialog = gtkam_chooser_new ();
	gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (m));
	gtk_widget_show (dialog);
	gtk_signal_connect (GTK_OBJECT (dialog), "camera_selected",
			    GTK_SIGNAL_FUNC (on_camera_selected), m);
}

static void
on_configure_activate (GtkMenuItem *item, GtkamMain *m)
{
	GtkWidget *dialog;

	if (!m->priv->camera)
		return;

	dialog = gtkam_config_new (m->priv->camera, m->priv->multi,
				   GTK_WIDGET (m));
	if (!dialog) {

		/* The error has already been reported */
		return;
	}

	gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (m));
	gtk_widget_show (dialog);
}

static void
on_configure_camera_clicked (GtkButton *button, GtkamMain *m)
{
	on_configure_activate (NULL, m);
}

static void
on_size_allocate (GtkWidget *widget, GtkAllocation *allocation, GtkamMain *m)
{
	gtk_icon_list_update (GTK_ICON_LIST (m->priv->list));
}

static void
gtkam_main_update_sensitivity (GtkamMain *m)
{
	CameraAbilities a;
	guint i, s;

	/* Make sure we are not shutting down */
	while (gtk_events_pending ())
		gtk_main_iteration ();
	if (!GTKAM_IS_MAIN (m))
		return;

	if (!m->priv->camera)
		return;

	gp_camera_get_abilities (m->priv->camera, &a);

	/* Select */
	i = g_list_length (GTK_ICON_LIST (m->priv->list)->icons);
	s = g_list_length (GTK_ICON_LIST (m->priv->list)->selection);
	gtk_widget_set_sensitive (m->priv->select_none, (s != 0));
	gtk_widget_set_sensitive (m->priv->select_all, (s != i));
	gtk_widget_set_sensitive (m->priv->select_inverse, (i != 0));
}

static void
on_folder_selected (GtkamTree *tree, Camera *camera, gboolean multi,
		    const gchar *folder, GtkamMain *m)
{
	/*
	 * Don't let the user switch folders while the list is downloading
	 * the file listing or thumbnails. If you want to give the user this
	 * possibility, you need to fix a reentrancy issue first.
	 */
	gtk_widget_set_sensitive (m->priv->vbox, FALSE);
	gtkam_list_add_folder (m->priv->list, camera, multi, folder);

	/* Again, make sure we aren't shutting down */
	if (!GTKAM_IS_MAIN (m))
		return;
	gtk_widget_set_sensitive (m->priv->vbox, TRUE);

	gtkam_main_update_sensitivity (m);
}

static void
on_folder_unselected (GtkTree *tree, Camera *camera, gboolean multi,
		      const gchar *folder, GtkamMain *m)
{
	gtkam_list_remove_folder (m->priv->list, camera, multi, folder);
	gtkam_main_update_sensitivity (m);
}

static void
on_debug_activate (GtkMenuItem *item, GtkamMain *m)
{
	GtkWidget *debug;

	debug = gtkam_debug_new ();
	gtk_widget_show (debug);
}

static void
on_about_activate (GtkMenuItem *item, GtkamMain *m)
{
	GtkWidget *dialog;
	char buf[4096];
	
	snprintf(buf, sizeof(buf), 
		 _("%s %s\n\n"
		   "gtKam was written by:\n"
		   " - Scott Fritzinger <scottf@unr.edu>,\n"
		   " - Lutz Mueller <urc8@rz.uni-karlsruhe.de>,\n"
		   " - and many others.\n"
		   "\n"
		   "gtKam uses libgphoto2, a library to access a\n"
		   "multitude of digital cameras. More \n"
		   "information is available at\n"
		   "http://www.gphoto.net.\n"
		   "\n"
		   "Enjoy the wonderful world of gphoto!"),
		 PACKAGE, VERSION);

	dialog = gtkam_close_new (buf, GTK_WIDGET (m));
	gtk_widget_show (dialog);
}

static gboolean
on_select_icon (GtkIconList *list, GtkIconListItem *item, GdkEvent *event,
		GtkamMain *m)
{
	guint i, s;

	gtkam_main_update_sensitivity (m);

	/*
	 * The problem is that the icon has not yet been selected. Therefore,
	 * we have to update the sensitivity manually.
	 */
	i = g_list_length (GTK_ICON_LIST (m->priv->list)->icons);
	s = g_list_length (GTK_ICON_LIST (m->priv->list)->selection) + 1;
	gtk_widget_set_sensitive (m->priv->select_none, (s != 0));
	gtk_widget_set_sensitive (m->priv->select_all, (s != i));
	gtk_widget_set_sensitive (m->priv->select_inverse, (i != 0)); 

	return (TRUE);
}

static void
on_unselect_icon (GtkIconList *list, GtkIconListItem *item, GdkEvent *event,
		  GtkamMain *m)
{
	gtkam_main_update_sensitivity (m);
}

static void
on_changed (GtkamList *list, GtkamMain *m)
{
	gtkam_main_update_sensitivity (m);
}

static void
on_new_status (GtkamTree *tree, GtkWidget *status, GtkamMain *m)
{
	gtk_box_pack_start (GTK_BOX (m->priv->status), status, FALSE, FALSE, 0);
}

static gboolean
load_tree (gpointer data)
{
	gtkam_tree_load (GTKAM_TREE (data));
	return (FALSE);
}

GtkWidget *
gtkam_main_new (void)
{
	GtkamMain *m;
	GtkWidget *vbox, *menubar, *menu, *item, *separator, *submenu;
	GtkWidget *frame, *scrolled, *check, *tree, *list, *label;
	GtkWidget *button, *hpaned, *toolbar, *icon;
	GtkAccelGroup *accel_group, *accels, *subaccels;
	GtkTooltips *tooltips;
	guint key;

	m = gtk_type_new (GTKAM_TYPE_MAIN);

	gtk_window_set_title (GTK_WINDOW (m), PACKAGE);
	gtk_window_set_default_size (GTK_WINDOW (m), 640, 480);
	gtk_window_set_policy (GTK_WINDOW (m), TRUE, TRUE, TRUE);
	gtk_signal_connect (GTK_OBJECT (m), "delete_event",
			    GTK_SIGNAL_FUNC (gtk_object_destroy), NULL);

	vbox = gtk_vbox_new (FALSE, 1);
	gtk_widget_show (vbox);
	gtk_container_add (GTK_CONTAINER (m), vbox);

	menubar = gtk_menu_bar_new ();
	gtk_widget_show (menubar);
	gtk_box_pack_start (GTK_BOX (vbox), menubar, FALSE, FALSE, 0);

	accel_group = gtk_accel_group_new ();
	tooltips = gtk_tooltips_new ();

	/*
	 * File menu
	 */
	item = gtk_menu_item_new_with_label ("");
	gtk_widget_show (item);
	key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (item)->child),
				     _("_File"));
	gtk_widget_add_accelerator (item, "activate_item", accel_group, key,
				    GDK_MOD1_MASK, 0);
	gtk_container_add (GTK_CONTAINER (menubar), item);

	menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu);
	accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (menu));

	item = gtk_menu_item_new_with_label ("");
	gtk_widget_show (item);
	gtk_widget_set_sensitive (item, FALSE);
	key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (item)->child),
				     _("_Save Selected Photos..."));
	gtk_widget_add_accelerator (item, "activate_item", accels, key, 0, 0);
	gtk_container_add (GTK_CONTAINER (menu), item);
	gtk_widget_add_accelerator (item, "activate", accels, GDK_s,
				    GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_signal_connect (GTK_OBJECT (item), "activate",
		GTK_SIGNAL_FUNC (on_save_selected_photos_activate), m);
	m->priv->item_save = item;

	item = gtk_menu_item_new_with_label ("");
	gtk_widget_show (item);
	key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (item)->child),
				     _("_Delete Photos"));
	gtk_widget_add_accelerator (item, "activate_item", accels, key, 0, 0);
	gtk_container_add (GTK_CONTAINER (menu), item);
	m->priv->menu_delete = item;

	submenu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), submenu);
	subaccels = gtk_menu_ensure_uline_accel_group (GTK_MENU (submenu));

	item = gtk_menu_item_new_with_label ("");
	gtk_widget_show (item);
	gtk_widget_set_sensitive (item, FALSE);
	key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (item)->child),
				     _("_Selected"));
	gtk_widget_add_accelerator (item, "activate_item", subaccels,
				    key, 0, 0);
	gtk_container_add (GTK_CONTAINER (submenu), item);
	gtk_signal_connect (GTK_OBJECT (item), "activate",
		GTK_SIGNAL_FUNC (on_delete_selected_photos_activate), m);
	m->priv->item_delete = item;

	item = gtk_menu_item_new_with_label ("");
	gtk_widget_show (item);
	key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (item)->child),
				     _("_All"));
	gtk_widget_add_accelerator (item, "activate_item", subaccels,
				    key, 0, 0);
	gtk_container_add (GTK_CONTAINER (submenu), item);
	gtk_signal_connect (GTK_OBJECT (item), "activate",
		GTK_SIGNAL_FUNC (on_delete_all_photos_activate), m);
	m->priv->item_delete_all = item;
	gtk_widget_set_sensitive (item, FALSE);

	separator = gtk_menu_item_new ();
	gtk_widget_show (separator);
	gtk_container_add (GTK_CONTAINER (menu), separator);
	gtk_widget_set_sensitive (separator, FALSE);

	item = gtk_menu_item_new_with_label ("");
	gtk_widget_show (item);
	key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (item)->child),
				     _("_Exit"));
	gtk_widget_add_accelerator (item, "activate_item", accels, key, 0, 0);
	gtk_signal_connect (GTK_OBJECT (item), "activate",
			    GTK_SIGNAL_FUNC (on_exit_activate), m);
	gtk_container_add (GTK_CONTAINER (menu), item);

	/*
	 * Select menu
	 */
	item = gtk_menu_item_new_with_label ("");
	gtk_widget_show (item);
	key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (item)->child),
				     _("_Select"));
	gtk_widget_add_accelerator (item, "activate_item", accel_group, key,
				    GDK_MOD1_MASK, 0);
	gtk_container_add (GTK_CONTAINER (menubar), item);

	menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu);
	accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (menu));

	item = gtk_menu_item_new_with_label ("");
	gtk_widget_show (item);
	gtk_widget_set_sensitive (item, FALSE);
	key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (item)->child),
				     _("_All"));
	gtk_widget_add_accelerator (item, "activate_item", accels, key, 0, 0);
	gtk_container_add (GTK_CONTAINER (menu), item);
	gtk_signal_connect (GTK_OBJECT (item), "activate",
			    GTK_SIGNAL_FUNC (on_select_all_activate), m);
	m->priv->select_all = item;

	item = gtk_menu_item_new_with_label ("");
	gtk_widget_show (item);
	gtk_widget_set_sensitive (item, FALSE);
	key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (item)->child),
				     _("_Inverse"));
	gtk_widget_add_accelerator (item, "activate_item", accels, key, 0, 0);
	gtk_container_add (GTK_CONTAINER (menu), item);
	gtk_signal_connect (GTK_OBJECT (item), "activate",
			    GTK_SIGNAL_FUNC (on_select_inverse_activate), m);
	m->priv->select_inverse = item;

	item = gtk_menu_item_new_with_label ("");
	gtk_widget_show (item);
	gtk_widget_set_sensitive (item, FALSE);
	key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (item)->child),
				     _("_None"));
	gtk_widget_add_accelerator (item, "activate_item", accels, key, 0, 0);
	gtk_container_add (GTK_CONTAINER (menu), item);
	gtk_signal_connect (GTK_OBJECT (item), "activate",
			    GTK_SIGNAL_FUNC (on_select_none_activate), m);
	m->priv->select_none = item;

	/*
	 * Camera menu
	 */
	item = gtk_menu_item_new_with_label ("");
	gtk_widget_show (item);
	key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (item)->child),
				     _("_Camera"));
	gtk_widget_add_accelerator (item, "activate_item", accel_group, key,
				    GDK_MOD1_MASK, 0);
	gtk_container_add (GTK_CONTAINER (menubar), item);

	menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu);
	accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (menu));

	item = gtk_menu_item_new_with_label ("");
	gtk_widget_show (item);
	key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (item)->child),
				     _("_Add Camera..."));
	gtk_widget_add_accelerator (item, "activate_item", accels, key, 0, 0);
	gtk_container_add (GTK_CONTAINER (menu), item);
	gtk_signal_connect (GTK_OBJECT (item), "activate",
			    GTK_SIGNAL_FUNC (on_add_camera_activate), m);

	/*
	 * Help menu
	 */
	item = gtk_menu_item_new_with_label ("");
	gtk_widget_show (item);
	key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (item)->child),
				     _("_Help"));
	gtk_widget_add_accelerator (item, "activate_item", accel_group,
				    key, 0, 0);
	gtk_container_add (GTK_CONTAINER (menubar), item);

	menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu);
	accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (menu));

	item = gtk_menu_item_new_with_label ("");
	gtk_widget_show (item);
	key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (item)->child),
				     _("_Debug..."));
	gtk_widget_add_accelerator (item, "activate_item", accels, key, 0, 0);
	gtk_container_add (GTK_CONTAINER (menu), item);
	gtk_signal_connect (GTK_OBJECT (item), "activate",
			    GTK_SIGNAL_FUNC (on_debug_activate), m);

	item = gtk_menu_item_new_with_label ("");
	gtk_widget_show (item);
	key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (item)->child),
				     _("_About..."));
	gtk_widget_add_accelerator (item, "activate_item", accels, key, 0, 0);
	gtk_container_add (GTK_CONTAINER (menu), item);
	gtk_signal_connect (GTK_OBJECT (item), "activate",
			    GTK_SIGNAL_FUNC (on_about_activate), m);

	/*
	 * Toolbar
	 */
	toolbar = gtk_toolbar_new (GTK_ORIENTATION_HORIZONTAL,
				   GTK_TOOLBAR_ICONS);
	gtk_widget_show (toolbar);
	gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, FALSE, 0);

	icon = create_pixmap (GTK_WIDGET (m), "save_current_image.xpm");
	button = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar),
			GTK_TOOLBAR_CHILD_BUTTON, NULL, NULL, NULL, NULL,
			icon, NULL, NULL);
	gtk_widget_show (button);
	gtk_tooltips_set_tip (tooltips, button, _("Save selected photos..."),
			      NULL);
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
		GTK_SIGNAL_FUNC (on_save_selected_photos_clicked), m);

	icon = create_pixmap (GTK_WIDGET (m), "delete_images.xpm");
	button = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar),
			GTK_TOOLBAR_CHILD_BUTTON, NULL, NULL, NULL, NULL,
			icon, NULL, NULL);
	gtk_widget_show (button);
	gtk_tooltips_set_tip (tooltips, button, _("Delete selected photos"),
			      NULL);
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
		GTK_SIGNAL_FUNC (on_delete_selected_photos_clicked), m);

	label = gtk_label_new ("      ");
	gtk_widget_show (label);
	gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), label, NULL, NULL);

	icon = create_pixmap (GTK_WIDGET (m), "configure.xpm");
	button = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar),
			GTK_TOOLBAR_CHILD_BUTTON, NULL, NULL, NULL, NULL,
			icon, NULL, NULL);
	gtk_widget_show (button);
	gtk_tooltips_set_tip (tooltips, button, _("Configure camera..."), NULL);
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
		GTK_SIGNAL_FUNC (on_configure_camera_clicked), m);

	label = gtk_label_new ("      ");
	gtk_widget_show (label);
	gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), label, NULL, NULL);

	icon = create_pixmap (GTK_WIDGET (m), "exit.xpm");
	button = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar),
			GTK_TOOLBAR_CHILD_BUTTON, NULL, NULL, NULL, NULL,
			icon, NULL, NULL);
	gtk_widget_show (button);
	gtk_tooltips_set_tip (tooltips, button, _("Exit"), NULL);
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
		GTK_SIGNAL_FUNC (on_exit_activate), m);

	/*
	 * Context information
	 */
	m->priv->status = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (m->priv->status);
	gtk_box_pack_end (GTK_BOX (vbox), m->priv->status, FALSE, FALSE, 0);

	/*
	 * Main content
	 */
	hpaned = gtk_hpaned_new ();
	gtk_widget_show (hpaned);
	gtk_box_pack_start (GTK_BOX (vbox), hpaned, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hpaned), 2);
	gtk_paned_set_position (GTK_PANED (hpaned), 200);

	/*
	 * Left
	 */
	vbox = gtk_vbox_new (FALSE, 5);
	gtk_widget_show (vbox);
	gtk_paned_pack1 (GTK_PANED (hpaned), vbox, FALSE, TRUE);
	m->priv->vbox = vbox;

	frame = gtk_frame_new (_("Index Settings"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);

	check = gtk_check_button_new_with_label (_("View Thumbnails"));
	gtk_widget_show (check);
	gtk_widget_set_sensitive (check, FALSE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), TRUE);
	gtk_container_add (GTK_CONTAINER (frame), check);
	gtk_signal_connect (GTK_OBJECT (check), "toggled",
			    GTK_SIGNAL_FUNC (on_thumbnails_toggled), m);
	m->priv->toggle_preview = GTK_TOGGLE_BUTTON (check);

	scrolled = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolled);
	gtk_box_pack_start (GTK_BOX (vbox), scrolled, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	tree = gtkam_tree_new ();
	gtk_widget_show (tree);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled),
					       tree);
	gtk_signal_connect (GTK_OBJECT (tree), "folder_selected",
			    GTK_SIGNAL_FUNC (on_folder_selected), m);
	gtk_signal_connect (GTK_OBJECT (tree), "folder_unselected",
			    GTK_SIGNAL_FUNC (on_folder_unselected), m);
	gtk_signal_connect (GTK_OBJECT (tree), "new_status",
			    GTK_SIGNAL_FUNC (on_new_status), m);
	m->priv->tree = GTKAM_TREE (tree);
	gtk_idle_add (load_tree, tree);

	/*
	 * Right
	 */
	scrolled = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolled);
	gtk_paned_pack2 (GTK_PANED (hpaned), scrolled, TRUE, TRUE);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	list = gtkam_list_new (m->priv->status);
	gtk_widget_show (list);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled),
					       list);
	m->priv->list = GTKAM_LIST (list);
	gtk_signal_connect (GTK_OBJECT (list), "changed",
			    GTK_SIGNAL_FUNC (on_changed), m);
	gtk_signal_connect (GTK_OBJECT (list), "select_icon",
			    GTK_SIGNAL_FUNC (on_select_icon), m);
	gtk_signal_connect (GTK_OBJECT (list), "unselect_icon",
			    GTK_SIGNAL_FUNC (on_unselect_icon), m);
	gtk_signal_connect (GTK_OBJECT (scrolled), "size_allocate",
			    GTK_SIGNAL_FUNC (on_size_allocate), m);

	return (GTK_WIDGET (m));
}

void
gtkam_main_set_camera (GtkamMain *m, Camera *camera, gboolean multi)
{
	CameraAbilities a;

	g_return_if_fail (GTKAM_IS_MAIN (m));
	g_return_if_fail (camera != NULL);

	if (m->priv->camera) {
		gp_camera_unref (m->priv->camera);
		m->priv->camera = NULL;
	}

	if (camera) {
		m->priv->camera = camera;
		gp_camera_ref (camera);
		m->priv->multi = multi;
	}

	gp_camera_get_abilities (camera, &a);

	/* Previews */
	if (camera && a.file_operations & GP_FILE_OPERATION_PREVIEW)
		gtk_widget_set_sensitive (GTK_WIDGET (m->priv->toggle_preview),
					  TRUE);
	else {
		gtk_widget_set_sensitive (GTK_WIDGET (m->priv->toggle_preview),
					  FALSE);
		gtk_toggle_button_set_active (m->priv->toggle_preview, FALSE);
	}

	/* Delete */
	if (camera && a.file_operations & GP_FILE_OPERATION_DELETE)
		gtk_widget_set_sensitive (m->priv->item_delete, TRUE);
	else
		gtk_widget_set_sensitive (m->priv->item_delete, FALSE);

	/* Delete all */
	if (camera && a.folder_operations & GP_FOLDER_OPERATION_DELETE_ALL)
		gtk_widget_set_sensitive (m->priv->item_delete_all, TRUE);
	else
		gtk_widget_set_sensitive (m->priv->item_delete_all, FALSE);

	/* Overall deletion */
	if (camera && ((a.file_operations & GP_FILE_OPERATION_DELETE) ||
		       (a.folder_operations & GP_FOLDER_OPERATION_DELETE_ALL)))
		gtk_widget_set_sensitive (m->priv->menu_delete, TRUE);
	else
		gtk_widget_set_sensitive (m->priv->menu_delete, FALSE);

	gtkam_main_update_sensitivity (m);

	/* The remaining */
	if (camera)
		gtk_widget_set_sensitive (m->priv->item_save, TRUE);
	else
		gtk_widget_set_sensitive (m->priv->item_save, FALSE);

	gtk_icon_list_clear (GTK_ICON_LIST (m->priv->list));
}
