/********************/
/* Type Definitions */
/********************/

typedef struct {
	Camera*			camera;
        CameraFile*		file;
	gchar*			folder;
	GnomeVFSOpenMode	mode;
        glong			position;
	GnomeVFSURI*		uri;
} file_handle_t;

typedef struct {
	GSList*			folders;
	GSList*			files;
	GnomeVFSFileInfoOptions	options;
        gint			position;
} directory_handle_t;

/**************/
/* Prototypes */
/**************/

GnomeVFSResult GNOME_VFS_RESULT (int result);

Camera*	camera_new_by_uri (GnomeVFSURI* uri, GSList* list, GnomeVFSContext* context, GnomeVFSResult* result);

GnomeVFSMethodHandle*	directory_handle_new (
				GnomeVFSURI* 		uri,
				GSList*			list,
				GnomeVFSFileInfoOptions options, 
				GnomeVFSContext*	context,
				GnomeVFSResult* 	result);
GnomeVFSResult		directory_handle_free (GnomeVFSMethodHandle* handle);

GnomeVFSMethodHandle*	file_handle_new	(
				GnomeVFSURI* 		uri, 
				GnomeVFSOpenMode 	mode, 
				GSList*			list,
				GnomeVFSContext*	context,
				GnomeVFSResult* 	result);
GnomeVFSResult		file_handle_free (GnomeVFSMethodHandle* handle);

