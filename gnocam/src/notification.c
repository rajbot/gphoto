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

extern GtkTree*		main_tree;

/*************/
/* Functions */
/*************/

void
on_camera_setup_changed (GConfClient* client, guint notify_id, GConfEntry* entry, gpointer user_data)
{
        if (entry->value == NULL) {

                /* No cameras configured. */
                camera_tree_update (main_tree, NULL);
        } else {
                camera_tree_update (main_tree, entry->value);
        }
}


