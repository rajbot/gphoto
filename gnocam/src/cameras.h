
/* Prototypes */

void camera_tree_folder_add 	(GtkTree* tree, Camera* camera, gchar* path);
void camera_tree_file_add 	(GtkTree* tree, Camera* camera, gchar* path, gchar* filename);

void camera_tree_folder_populate		(GtkTreeItem* folder);
void camera_tree_folder_clean 			(GtkTreeItem* folder);
void camera_tree_folder_refresh 		(GtkTreeItem* folder);
void camera_tree_folder_notebook_pages_remove 	(GtkTreeItem* folder);

void camera_tree_item_remove 	(GtkTreeItem* item);

void camera_tree_update (GtkTree* tree, GConfValue* value);

void update_pixmap (GtkPixmap* pixmap, CameraFile* file);

void camera_tree_update_pixmaps (GtkTree* tree);

