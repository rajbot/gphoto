
/* Prototypes */

void save_all_selected (GtkTree* tree, gboolean file, gboolean prompt_for_filename, gboolean temporary);

void delete_all_selected (GtkTree* tree);

void save (GladeXML* xml, Camera* camera, gchar* path, gchar* filename, gboolean file, gboolean temporary);
void save_as (GladeXML* xml, Camera* camera, gchar* path, gchar* filename, gboolean file);

void delete (GtkTreeItem* item);

void upload (Camera* camera, gchar* path_orig, gchar* filename);

