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

	g_assert ((tree = GTK_TREE (glade_xml_get_widget (xml, "tree_cameras"))) != NULL);

	/* Redraw the pixmaps in the notebook. */
        camera_tree_rescale_pixmaps (GTK_TREE (glade_xml_get_widget (xml, "tree_cameras")));
}


