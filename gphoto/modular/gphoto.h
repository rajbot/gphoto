/* This contains the things that Libraries will want access to.
 * Since they are not #included in main.c any more they need some
 * other way of finding this lot out. The overheads of main.h seemed
 * silly.
 */ 

#include <gdk_imlib.h>
#include <gtk/gtk.h>

typedef int 		(*_initialize)			();
typedef GdkImlibImage* 	(*_get_picture)			(int,int);
typedef GdkImlibImage*	(*_get_preview)			();
typedef int 		(*_delete_picture)		(int);
typedef int 		(*_take_picture)		();
typedef int 		(*_number_of_pictures)		();
typedef int 		(*_configure)			();
typedef char*		(*_summary)			();
typedef char*		(*_description)			();

struct _Camera {
	_initialize	initialize;
	_get_picture	get_picture;
	_get_preview 	get_preview;
	_delete_picture delete_picture;
	_take_picture	take_picture;
	_number_of_pictures number_of_pictures;
	_configure	configure;
	_summary	summary;
	_description	description;
};

struct Model {
	char *name;
	struct _Camera *library;
};

struct ModelMod {
  char *name,*libname,*structname; /* we shouldn't need 3 names here...*/
};

struct ImageInfo {
        GdkImlibImage *imlibimage;
        GtkWidget *image;
        GtkWidget *button;
        GtkWidget *page;
        GtkWidget *label;
        char      *info;
        struct    ImageInfo *next;
};

extern int	  picCounter;		/* Whenever gPhoto needs a #	*/
extern char      *gphotoDir;		/* gPhoto directory		*/
extern char	  serial_port[80];	/* Serial port			*/
extern char	  camera_model[100];	/* Currently selected cam model */

extern GtkWidget *status_bar;           /* Main window status bar       */
extern GtkWidget *library_name; /* Main window library bar      */
extern GtkWidget *notebook;            /* Main window Notebook          */
extern GtkWidget *index_table;         /* Index table                   */
extern GtkWidget *index_vp;            /* Viewport for above           */
extern GtkWidget *index_window; /* Index Scrolled Window        */
extern GtkWidget *progress;             /* Progress bar                 */
