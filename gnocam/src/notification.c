#include <gnome.h>
#include <glade/glade.h>
#include <gconf/gconf-client.h>
#include <gphoto2.h>
#include <bonobo.h>
#include <libgnomevfs/gnome-vfs.h>
#include "gnocam.h"
#include "cameras.h"
#include "notification.h"

/*************/
/* Functions */
/*************/

void
on_camera_setup_changed (GConfClient* client, guint notify_id, GConfEntry* entry, gpointer user_data)
{
	main_tree_update (entry->value);
}

