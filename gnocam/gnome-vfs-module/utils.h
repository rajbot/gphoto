#define GNOCAM_VFS_DEBUG 1
#if GNOCAM_VFS_DEBUG
#define CAM_VFS_DEBUG_PRINT(x)				\
G_STMT_START {                                          \
        printf ("%s:%d ", __FILE__,__LINE__);		\
        printf ("%s() ", __FUNCTION__);			\
        printf x;					\
        fputc ('\n', stdout);				\
        fflush (stdout);				\
}G_STMT_END
#define CAM_VFS_DEBUG(x) CAM_VFS_DEBUG_PRINT(x)
#else
#define CAM_VFS_DEBUG(x)
#endif

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

GnomeVFSMethodHandle*	directory_handle_new (
				GnomeVFSURI* 		uri,
				GnomeVFSFileInfoOptions options, 
				GnomeVFSContext*	context,
				GnomeVFSResult* 	result);
GnomeVFSResult		directory_handle_free (GnomeVFSMethodHandle* handle);

GnomeVFSMethodHandle*	file_handle_new	(
				GnomeVFSURI* 		uri, 
				GnomeVFSOpenMode 	mode, 
				GnomeVFSContext*	context,
				GnomeVFSResult* 	result);
GnomeVFSResult		file_handle_free (GnomeVFSMethodHandle* handle);

