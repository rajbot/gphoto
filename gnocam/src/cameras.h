
/* Prototypes */

void camera_tree_folder_add 		(GtkTree* tree, Camera* camera, gchar* url);
void camera_tree_folder_populate	(GtkTreeItem* folder);
void camera_tree_folder_clean 		(GtkTreeItem* folder);

void camera_tree_file_add       	(GtkTree* tree, gchar* url);

void camera_tree_item_popup_create 	(GtkTreeItem* item);

void main_tree_update (void);



