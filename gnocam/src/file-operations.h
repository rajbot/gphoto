
/* Prototypes */

void save_all_selected 		(GtkTree* tree, gboolean preview, gboolean save_as);

void save 	(GtkTreeItem* item, gboolean preview, gboolean save_as);
void upload 	(GtkTreeItem* item, GnomeVFSURI* uri);
void download 	(GtkTreeItem* item, GnomeVFSURI* uri, gboolean preview);
void delete	(GtkTreeItem* item);

void camera_file_save (CameraFile* file, GnomeVFSURI* uri);



