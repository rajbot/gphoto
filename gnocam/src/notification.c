#include <gnome.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include <gphoto2.h>
#include "gnocam.h"
#include "cameras.h"
#include "notification.h"

/**********************/
/* External Variables */
/**********************/

extern GladeXML*	xml;

/*************/
/* Functions */
/*************/

void
on_camera_setup_changed (GConfClient* client, guint notify_id, GConfEntry* entry, gpointer user_data)
{
        GtkTree*        tree;

        g_assert ((tree = GTK_TREE (glade_xml_get_widget (xml, "tree_cameras"))) != NULL);

        if (entry->value == NULL) {

                /* No cameras configured. */
                camera_tree_update (tree, NULL);
        } else {
                camera_tree_update (tree, entry->value);
        }
}

void
on_preview_setup_changed (GConfClient* client, guint notify_id, GConfEntry* entry, gpointer user_data)
{
	GtkTree*		tree;
	gint			i;
	Camera*			camera;
	frontend_data_t*	frontend_data;
	GtkPixmap*		pixmap;
	CameraFile*		file;

	g_assert ((tree = GTK_TREE (glade_xml_get_widget (xml, "tree_cameras"))) != NULL);

	/* Redraw the pixmaps in the notebook. */
        camera_tree_rescale_pixmaps (GTK_TREE (glade_xml_get_widget (xml, "tree_cameras")));

	/* Redraw the pixmaps in the preview apps (if any). */
	for (i = 0; i < g_list_length (tree->children); i++) {
		g_assert ((camera = gtk_object_get_data (GTK_OBJECT (g_list_nth_data (tree->children, i)), "camera")) != NULL);
		g_assert ((frontend_data = (frontend_data_t*) camera->frontend_data) != NULL);
		if (frontend_data->xml_preview) {
			g_assert ((pixmap = GTK_PIXMAP (glade_xml_get_widget (frontend_data->xml_preview, "app_preview_pixmap"))) != NULL);
			if ((file = gtk_object_get_data (GTK_OBJECT (glade_xml_get_widget (frontend_data->xml_preview, "app_preview")), "file")))
				pixmap_rescale (pixmap, NULL);
		}
	}
}


