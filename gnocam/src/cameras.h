
/* Prototypes */

void camera_tree_folder_add (GtkTree* tree, Camera* camera, gchar* path);

void camera_tree_folder_clean (GtkTreeItem* item);

void camera_tree_item_remove (GtkTreeItem* item);

void camera_tree_update (GtkTree* tree, GConfValue* value);

void camera_tree_item_update_pixmap (GtkTreeItem* item);
void camera_tree_update_pixmaps (GtkTree* tree);

