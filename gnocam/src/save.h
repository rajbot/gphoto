
/* Prototypes */

void save_all_selected (GladeXML* xml, gboolean file, gboolean prompt_for_filename, gboolean temporary);

void save (GladeXML* xml, Camera* camera, gchar* path, gchar* filename, gboolean file, gboolean temporary);
void save_as (GladeXML* xml, Camera* camera, gchar* path, gchar* filename, gboolean file);

