
/* Prototypes */

void save_all_selected 		(GtkTree* tree, gboolean preview, gboolean save_as, gboolean temporary);
void delete_all_selected 	(GtkTree* tree);

void delete	(GtkTreeItem* item);
void save 	(GtkTreeItem* item, gboolean preview, gboolean save_as, gboolean temporary);
void upload 	(GtkTreeItem* item, gchar* filename);

void camera_file_save (CameraFile* file, GnomeVFSURI* uri);
void camera_file_save_as (CameraFile* file);
void camera_file_upload (Camera* camera, gchar* path, CameraFile* file);



