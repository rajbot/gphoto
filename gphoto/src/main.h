/* All the happy includes, typedefs, and evil globals.. :)
 * 17/3/99 - Some of the main struct defs and externs have been
 * moved to gphoto.h Dynamic Libraries will want to use these and the
 * unecessary overheads in here are best kept separate. - Phill 
 */

#include <gtk/gtk.h>

/* Big happy title and version # */
#define   TITLE_VER	"GNU Photo (gPhoto) - v0.3"

extern struct Model cameras[];

#ifdef  GTK_HAVE_FEATURES_1_1_0
extern GtkAccelGroup*  mainag;
#endif



int	  picCounter;		/* Whenever gPhoto needs a #	*/
char      *gphotoDir;		/* gPhoto directory		*/
char	  serial_port[20];	/* Serial port			*/
char	  camera_model[100];	/* Currently selected cam model */

GtkWidget *status_bar;		/* Main window status bar	*/
GtkWidget *library_name;	/* Main window library bar	*/
GtkWidget *notebook;            /* Main window Notebook		*/
GtkWidget *index_table;         /* Index table			*/
GtkWidget *index_vp;            /* Viewport for above           */
GtkWidget *index_window;	/* Index Scrolled Window        */
GtkWidget *progress;		/* Progress bar			*/

#ifdef  GTK_HAVE_FEATURES_1_1_0
GtkAccelGroup*  mainag;
#endif

struct _Camera  *Camera;
struct ImageInfo Images;
struct ImageInfo Thumbnails;

