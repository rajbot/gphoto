/********************/
/* Type Definitions */
/********************/

typedef struct {
        CameraFile*     file;
        glong           position;
} file_handle_t;

typedef struct {
	GSList*		folders;
	GSList*		files;
        gint		position;
} directory_handle_t;

/**************/
/* Prototypes */
/**************/

Camera*	camera_new_by_uri (GnomeVFSURI* uri, GnomeVFSResult* result);

GnomeVFSMethodHandle*	directory_handle_new 	(GnomeVFSURI* uri, GnomeVFSResult* result);
GnomeVFSResult		directory_handle_free	(GnomeVFSMethodHandle* handle);

GnomeVFSMethodHandle*	file_handle_new		(GnomeVFSURI* uri, GnomeVFSResult* result);
GnomeVFSResult		file_handle_free	(GnomeVFSMethodHandle* handle);

