/* All the happy includes, typedefs, and evil globals.. :)
 * 17/3/99 - Some of the main struct defs and externs have been
 * moved to gphoto.h Dynamic Libraries will want to use these and the
 * unecessary overheads in here are best kept separate. - Phill 
 */

#include <gtk/gtk.h>

/* Big happy title and version # */
#define   TITLE_VER	"GNU Photo (gPhoto) - v0.3 DLV (Dynamic Library Version)"

extern struct ModelMod cameras[];

#ifdef  GTK_HAVE_FEATURES_1_1_0
extern GtkAccelGroup*  mainag;
#endif

/* Prototypes (most anyway)  -------------- */

void update_status(char *newStatus);
				/* Sets the status bar text	*/

void update_progress(float percentage);
				/* Sets the progress bar	*/

int wait_for_hide (GtkWidget *dialog, GtkWidget *ok_button,
                   GtkWidget *cancel_button);
				/* waits for dialog to be hidden */

void error_dialog(char *Error);
				/* Simple Dialog w/ OK button	*/

void configure_call();
				/* Calls the camera config	*/

void del_dialog();
				/* Delete pictures confirmation	*/

void appendpic (int picNum, int fromCamera, char *fileName);
				/* Appends a picture to the notebook */

void destroy (GtkWidget *widget, gpointer data);
				/* gtk Destroy function		*/

void save_port(GtkWidget *widget, GtkWidget *dialog);
				/* Saves port dialog selection	*/

void port_dialog();
				/* Port/camera dialog		*/

void version_dialog();
				/* Version dialog		*/

void usersmanual_dialog();
				/* Manual dialog		*/

void faq_dialog();
				/* FAQ dialog			*/

void about_dialog();
				/* About dialog			*/

void menu_selected (gchar *toPrint);
				/* Says "blah not implemented"	*/

void remove_thumbnail (int i);
				/* removes thumbnail from linked list */

void getindex ();
				/* retrieves the index		*/

void getpics ();
				/* gets the selected pictures	*/

void remove_image(int i);
				/* kills image from linked list	*/

void closepic ();
				/* closes currently opened pic 	*/

void savepic (GtkWidget *widget, GtkFileSelection *fwin);
				/* saves currently viewed pic	*/

void openpic (GtkWidget *widget, GtkFileSelection *fwin);
				/* loads pic from disk		*/

void filedialog (gchar *a);
				/* generic file-dialog function */

void ok_click (GtkWidget *dialog);
				/* read the comments for info	*/

void send_to_gimp ();
				/* sends current pic to gimp	*/

void print_pic ();
				/* prints current pic		*/

void select_all();
				/* selects all index thumbnails	*/

void select_none();
				/* selects no index thumbnails	*/

void select_inverse();
				/* selects opposite thumbnails 	*/

