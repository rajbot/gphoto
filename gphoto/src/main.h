/* All the happy includes, typedefs, and evil globals.. :)
 * 17/3/99 - Some of the main struct defs and externs have been
 * moved to gphoto.h Dynamic Libraries will want to use these and the
 * unecessary overheads in here are best kept separate. - Phill 
 */

#include <gtk/gtk.h>

/* Big happy title and version # */
#define   TITLE_VER	"GNU Photo (gPhoto) - v0.3"

extern struct Model cameras[];
extern char filesel_cwd[];

#ifdef  GTK_HAVE_FEATURES_1_1_0
extern GtkAccelGroup*  mainag;
#endif



       int	  picCounter;		/* Whenever gPhoto needs a #	*/
       char	  camera_model[100];	/* Currently selected cam model */
extern int 	  command_line_mode;
extern char      *gphotoDir;		/* gPhoto directory		*/
extern char	  serial_port[20];	/* Serial port			*/

GtkWidget *status_bar;		/* Main window status bar	*/
GtkWidget *library_name;	/* Main window library bar	*/
GtkWidget *notebook;            /* Main window Notebook		*/
GtkWidget *index_table;         /* Index table			*/
GtkWidget *index_vp;            /* Viewport for above           */
GtkWidget *index_window;	/* Index Scrolled Window        */
GtkWidget *progress;		/* Progress bar			*/

int	   post_process;	/* TRUE/FALSE to post-process   */
char	   post_process_script[1024]; /* Full path/filename	*/
GtkWidget *post_process_pixmap; /* Post process pixmap		*/

#ifdef  GTK_HAVE_FEATURES_1_1_0
GtkAccelGroup*  mainag;
#endif

struct _Camera  *Camera;
struct ImageInfo Images;
struct ImageInfo Thumbnails;

