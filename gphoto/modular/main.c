#include "main.h"
#include "gphoto.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef linux
#include <sched.h>
#endif

/* The other stuff that main.c needs */

#include "callbacks.h"
#include "cameras.h"
#include "menu.h"
#include "toolbar.h"
#include "commandline.h"

#include "splash.xpm"		/* Splash screen  */

int	  picCounter=1;		/* Whenever gPhoto needs a #	*/
char      *gphotoDir;		/* gPhoto directory		*/
char	  serial_port[80];	/* Serial port			*/
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

struct _Camera *Camera;
struct ImageInfo Images;
struct ImageInfo Thumbnails;

int main (int argc, char *argv[]) {

	int no_rc=0;

	GtkWidget *mainWin;
	GtkWidget *table;
	GtkWidget *menu_bar;
	GtkWidget *index_page;
	GtkWidget *label, *box, *sbox;
	GtkWidget *vseparator;
	GtkStyle *style;

	GtkWidget *gpixmap;
	GdkPixmap *pixmap;
	GdkBitmap *bitmap;
	FILE *conf;
	char fname[1024];

	gtk_init(&argc, &argv);
	gdk_imlib_init();
	gtk_widget_push_visual(gdk_imlib_get_visual());
	gtk_widget_push_colormap(gdk_imlib_get_colormap());

	Thumbnails.next = NULL;
	Images.next=NULL;

	/* Set the priority (taken from PhotoPC photopc.c) */

#ifdef linux
        if (geteuid() == 0) {
                struct sched_param sp;
                int rc,minp,maxp;

                minp=sched_get_priority_min(SCHED_FIFO);
                maxp=sched_get_priority_max(SCHED_FIFO);
                sp.sched_priority=minp+(maxp-minp)/2;
                if ((rc=sched_setscheduler(0,SCHED_FIFO,&sp)) == -1)
                        fprintf(stderr,"failed to set priority\n");
                /* (void)seteuid(getuid()); */
	}
#endif

	/* Make sure there's a .gphoto directory in their home ---- */
	gphotoDir = getenv("HOME");
	sprintf(gphotoDir, "%s/.gphoto", gphotoDir);
	(void)mkdir(gphotoDir, 0744);

	/* Load config options------------------------------------ */
	sprintf(fname, "%s/gphotorc", gphotoDir);
	conf = fopen(fname, "r");
	if (!conf)
		no_rc = 1;
	   else {
		fgets(fname, 100, conf);
		strncpy(serial_port, fname, strlen(fname)-1);
		fgets(fname, 100, conf);
		strncpy(camera_model, fname, strlen(fname)-1);
		fclose(conf);
	}

	library_name = gtk_label_new("");
	set_camera(camera_model);

	/* Command line mode anyone? ----------------------------- */
	if (argc > 1)
		command_line(argc, argv);

	/* set up the main window -------------------------------- */
	mainWin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_container_border_width (GTK_CONTAINER(mainWin), 0);
	gtk_window_set_title (GTK_WINDOW(mainWin), TITLE_VER);
	gtk_signal_connect (GTK_OBJECT(mainWin), "delete_event",
			    GTK_SIGNAL_FUNC(delete_event), NULL);
	gtk_widget_set_usize(mainWin, 820, 480);
	gtk_widget_realize(mainWin);

	/* set up the menu --------------------------------------- */
	menu_bar = gtk_menu_bar_new();
	create_menu(menu_bar);
	gtk_widget_show(menu_bar);

	/* button bar -------------------------------------------- */
	box = gtk_hbox_new(FALSE, 0);
	create_toolbar(box, mainWin);
	gtk_widget_show(box);
	gtk_container_border_width(GTK_CONTAINER(box), 5);

	/* accelerator keys--------------------------------------- */
#ifdef  GTK_HAVE_FEATURES_1_1_0
	gtk_accel_group_attach(mainag,GTK_OBJECT(mainWin));
#endif

	/* Index Page notebook ----------------------------------- */
	index_page = gtk_table_new(1,1,FALSE);
	gtk_widget_show(index_page);
	index_window = gtk_scrolled_window_new(NULL,NULL);
#ifdef  GTK_HAVE_FEATURES_1_1_4
        index_vp=gtk_viewport_new(NULL,NULL);
        gtk_container_add(GTK_CONTAINER(index_window),index_vp);
        gtk_widget_show(index_vp);
#endif
	gtk_widget_show(index_window);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(index_window),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_table_attach_defaults(GTK_TABLE(index_page),index_window,0,1,0,1);

	notebook = gtk_notebook_new();
	gtk_widget_show(notebook);
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);

	label = gtk_label_new("Image Index");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), index_page,
				 label);

	sbox = gtk_hbox_new(FALSE, 5);
	gtk_widget_show(sbox);

	status_bar = gtk_label_new("");
	gtk_widget_show(status_bar);
	gtk_label_set_justify(GTK_LABEL(status_bar), GTK_JUSTIFY_LEFT);	
	gtk_box_pack_start(GTK_BOX(sbox), status_bar, FALSE, FALSE, 0);
	update_status("Select \"Camera->Get Index\" to preview images.");

	progress = gtk_progress_bar_new();
	gtk_widget_show(progress);
	gtk_widget_set_usize(progress, 100, 16);
	gtk_box_pack_end(GTK_BOX(sbox), progress, FALSE, FALSE, 0);

	gtk_widget_show(library_name);
	gtk_widget_set_usize(library_name, 200, 16);
	gtk_label_set_justify(GTK_LABEL(library_name), GTK_JUSTIFY_LEFT);
	gtk_box_pack_end(GTK_BOX(sbox), library_name, FALSE, FALSE, 0);

	vseparator = gtk_vseparator_new();
	gtk_widget_show(vseparator);
	gtk_box_pack_end(GTK_BOX(sbox), vseparator, FALSE, FALSE, 0);

	style = gtk_widget_get_style(mainWin);
	pixmap = gdk_pixmap_create_from_xpm_d(mainWin->window, &bitmap,
					&style->bg[GTK_STATE_NORMAL],
					(gchar **)splash_xpm);
	gpixmap = gtk_pixmap_new(pixmap, bitmap);
	gtk_widget_show(gpixmap);

	/* Main window layout ------------------------------------ */
	table =gtk_table_new(4,1,FALSE);
	gtk_container_add(GTK_CONTAINER(mainWin), table);
	gtk_widget_show(table);
	gtk_table_attach(GTK_TABLE(table),menu_bar, 0, 1, 0, 1,
			 GTK_FILL|GTK_EXPAND, GTK_FILL, 0 , 0);
	gtk_table_attach(GTK_TABLE(table), box, 0, 1, 1, 2,
			 GTK_FILL|GTK_EXPAND, GTK_FILL, 0, 0);
	gtk_table_attach_defaults(GTK_TABLE(table),notebook, 0, 1, 2, 3);
	gtk_table_attach(GTK_TABLE(table),sbox, 0, 1, 3, 4,
			 GTK_FILL|GTK_EXPAND, GTK_FILL, 0 , 0);

	index_table = gtk_hbox_new(FALSE, 0);
        gtk_widget_show(index_table);
#ifndef GTK_HAVE_FEATURES_1_1_4
       gtk_container_add( GTK_CONTAINER(index_window), index_table); 
#else
       gtk_container_add( GTK_CONTAINER(index_vp), index_table); 
#endif

	gtk_box_pack_start(GTK_BOX(index_table), gpixmap, TRUE, FALSE, 0);

	/* If not command-line mode... --------------------------- */
	gtk_widget_show(mainWin);
	if (no_rc)
		port_dialog();
	gtk_main();
	return(0);
}
