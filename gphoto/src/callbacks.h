extern struct ImageInfo Thumbnails;
extern struct ImageInfo Images;
extern struct _Camera *Camera;

/* Helper Functions ------------------------------------------
   ----------------------------------------------------------- */
void set_camera (char *model);

void configure_call();

void takepicture_call();

void del_pics (GtkWidget *dialog, GtkObject *button);

void del_dialog ();

void appendpic (int picNum, int fromCamera, char *fileName);

/* GTK Functions ---------------------------------------------
   ----------------------------------------------------------- */

gint delete_event (GtkWidget *widget, GdkEvent *event, gpointer data);

/* Callbacks -------------------------------------------------
   ----------------------------------------------------------- */

void port_dialog();

void version_dialog();
void faq_dialog();
void about_dialog();
void show_license();
void usersmanual_dialog();

/* used until callback is implemented */
void menu_selected (gchar *toPrint);

void remove_thumbnail (int i);

/* Place a thumbnail into the index at location corresponding to node */
void insert_thumbnail(struct ImageInfo *node);

/* intercept mouse click on a thumbnail button */
gint thumb_click( GtkWidget *widget,GdkEventButton *event,gpointer callback_data );


/* get index of images and place in main page table */

void getindex ();

/* get empty index of images and place in main page table */
void getindex_empty ();

/* get selected pictures */
void getpics ();

void remove_image(int i);

void closepic ();

void saveselectedtodisk ();

void savepic (GtkWidget *widget, GtkFileSelection *fwin);

/* save picture being viewed */
void filedialog (gchar *a);

void print_pic ();

void select_all();

void select_inverse();

void select_none();

void resize_dialog();

void color_dialog();

void manip_pic (gchar *Option);

void summary_dialog();

void scale (int factor);

void scale_half ();

void scale_double ();

extern GtkWidget * img_edit_new(struct ImageInfo *ii);

/* For use with the Menu Factory */

/* I will port the toolbar to the toolbar factory as well for common callbacks 	*/
/* the callbacks need to be rehashed to be a little less confusing name-wise 	*/
/*						-Scott				*/

void save_images (gpointer data, guint action, GtkWidget *widget);
		/* Saves all the selected images to disk */

void open_images (gpointer data, guint action, GtkWidget *widget);
		/* Opens all the selected images */

void save_thumbs (gpointer data, guint action, GtkWidget *widget);
		/* Saves all the selected thumbnails to disk */

void open_thumbs (gpointer data, guint action, GtkWidget *widget);
		/* Opens all the selected thumbnails */

void save_both   (gpointer data, guint action, GtkWidget *widget);
		/* Saves all the selected images & thumbnails to disk */

void open_both   (gpointer data, guint action, GtkWidget *widget);
		/* Saves all the selected images & thumbnails to disk */

