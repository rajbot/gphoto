
/* Prototypes */

void camera_tree_folder_add 		(GtkTree* tree, Camera* camera, gchar* path);
void camera_tree_folder_populate	(GtkTreeItem* folder);
void camera_tree_folder_clean 		(GtkTreeItem* folder);
void camera_tree_folder_refresh 	(GtkTreeItem* folder);

void camera_tree_file_add       	(GtkTree* tree, Camera* camera, gchar* path, gchar* filename);

void camera_tree_item_remove 		(GtkTreeItem* item);

void camera_tree_update (GtkTree* tree, GConfValue* value);

void pixmap_set 	(GtkPixmap* pixmap, CameraFile* file);
void pixmap_rescale 	(GtkPixmap* pixmap, gfloat* magnification);

void camera_tree_rescale_pixmaps (GtkTree* tree);

void app_clean_up (void);

void page_remove (GladeXML* xml_page);

