
/* Prototypes */

void save_all_selected 		(GtkTree* tree, gboolean preview, gboolean save_as, gboolean temporary);

void save 	(GtkTreeItem* item, gboolean preview, gboolean save_as, gboolean temporary);
void upload 	(GtkTreeItem* item, gchar* filename);
void delete	(GtkTreeItem* item);

void camera_file_save (CameraFile* file, GnomeVFSURI* uri);
void camera_file_save_as (CameraFile* file);



