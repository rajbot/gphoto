
extern struct ImageInfo Thumbnails;
extern struct ImageInfo Images;
extern struct _Camera *Camera;

/* Helper Functions ------------------------------------------
   ----------------------------------------------------------- */
void update_status(char *newStatus);

	/*
		displays whatever is in string "newStatus" in the
		status bar at the bottom of the main window
	*/



void update_progress(float percentage);
	/*
		sets the progress bar to percentage% at the bottom
		of the main window
	*/

void error_dialog(char *Error);
	/* 
	   Standard, run-of-the-mill message box
	*/

void ok_click (GtkWidget *dialog);

int wait_for_hide (GtkWidget *dialog, GtkWidget *ok_button,
		   GtkWidget *cancel_button) ;

void set_camera (char *model);

void configure_call();

void takepicture_call();

void del_pics (GtkWidget *dialog);

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

void batch_save_dialog();

/* save picture being viewed as jpeg */
void filedialog (gchar *a);

void send_to_gimp ();

void print_pic ();

void select_all();

void select_inverse();

void select_none();

void resize_dialog();

void manip_pic (gchar *Option);

void summary_dialog();

