
/* Prototypes */

void camera_tree_folder_add (GtkTree* tree, Camera* camera, gchar* path);

void camera_tree_update (GtkTree* tree, GConfValue* value);

void camera_tree_clean (GtkTree* tree);

void camera_tree_remove_file (GtkTreeItem* item);

