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

Camera*	camera_new_by_uri (GnomeVFSURI* uri, GConfClient* client, GMutex* client_mutex, GnomeVFSContext* context, GnomeVFSResult* result);

GnomeVFSMethodHandle*	directory_handle_new (
				GnomeVFSURI* 		uri, 
				GConfClient* 		client, 
				GMutex* 		client_mutex, 
				GnomeVFSFileInfoOptions options, 
				GnomeVFSContext*	context,
				GnomeVFSResult* 	result);
GnomeVFSResult		directory_handle_free (GnomeVFSMethodHandle* handle);

GnomeVFSMethodHandle*	file_handle_new	(
				GnomeVFSURI* 		uri, 
				GnomeVFSOpenMode 	mode, 
				GConfClient* 		client, 
				GMutex* 		client_mutex, 
				GnomeVFSContext*	context,
				GnomeVFSResult* 	result);
GnomeVFSResult		file_handle_free (GnomeVFSMethodHandle* handle);

