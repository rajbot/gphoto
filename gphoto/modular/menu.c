#include "main.h"
#include "gphoto.h"
#include "menu.h"

#include "callbacks.h"
#include "gallery.h"
#include "live.h"


/* Build the Menu --------------------------------------------
   ----------------------------------------------------------- */
#ifdef  GTK_HAVE_FEATURES_1_1_0
GtkAccelGroup*  mainag;
#endif

void add_to_menu (gchar *label, GtkSignalFunc f, gpointer data, 
		  GtkWidget *menu) {

	GtkWidget *menu_item;

	if (strcmp(label, "") == 0)
		menu_item = gtk_menu_item_new();
	   else {
		menu_item = gtk_menu_item_new_with_label(label);
		gtk_signal_connect_object (GTK_OBJECT(menu_item), 
					   "activate", f, data);
	}
	gtk_menu_append (GTK_MENU(menu), menu_item);
	gtk_widget_show(menu_item);
}

/* 
   Add a menu entry with a keyboard accelerator.
      note that a modifier of 0 defines an unmodified keystroke
 */
void add_to_menu_acc (gchar *label, GtkSignalFunc f, gpointer data, 
		  GtkWidget *menu, char akey,GdkModifierType modmask) {

	GtkWidget *menu_item;

	if (strcmp(label, "") == 0)
		menu_item = gtk_menu_item_new();
	   else {
		menu_item = gtk_menu_item_new_with_label(label);
		gtk_signal_connect_object (GTK_OBJECT(menu_item), 
					   "activate", f, data);
	}
	gtk_menu_append (GTK_MENU(menu), menu_item);
#ifdef  GTK_HAVE_FEATURES_1_1_0
	gtk_accel_group_add(mainag,akey,modmask,
			  GTK_ACCEL_VISIBLE||GTK_ACCEL_LOCKED,
			  GTK_OBJECT(menu_item),"activate");
#endif
	gtk_widget_show(menu_item);
}

void create_menu (GtkWidget *menu_bar) {

	GtkWidget *menu, *amenu, *root_menu;
#ifdef  GTK_HAVE_FEATURES_1_1_0
	mainag=gtk_accel_group_new();
#endif
	
	/* File Menu ----------------------------------------------- */
	menu = gtk_menu_new ();
	add_to_menu_acc("Open", GTK_SIGNAL_FUNC(filedialog),
		    (gpointer)"o", menu,'o',GDK_CONTROL_MASK);
	add_to_menu_acc("Save Current Image...", GTK_SIGNAL_FUNC(filedialog),
		    (gpointer)"s", menu,'s',GDK_CONTROL_MASK);
	add_to_menu("Batch Save...", GTK_SIGNAL_FUNC(batch_save_dialog),
		    (gpointer)"Batch Save", menu);

/* 	Please see comments in callbacks.c for help...
	add_to_menu("Send to GIMP", GTK_SIGNAL_FUNC(send_to_gimp),
		    (gpointer)"SendToGimp", menu);
*/

	add_to_menu("", NULL,NULL, menu);
	add_to_menu("Print", GTK_SIGNAL_FUNC(print_pic),
		    (gpointer)"Print", menu);
	add_to_menu("", NULL,NULL, menu);
	add_to_menu_acc("Close", GTK_SIGNAL_FUNC(closepic),
		    (gpointer)"Close", menu,'c',GDK_CONTROL_MASK);
	add_to_menu_acc("Exit", GTK_SIGNAL_FUNC(delete_event),
		    (gpointer)"Exit", menu,'q',GDK_CONTROL_MASK);
	root_menu = gtk_menu_item_new_with_label("File");
	gtk_widget_show(root_menu);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM(root_menu), menu);	
	gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), root_menu);

	/* Edit Menu ----------------------------------------------- */
	menu = gtk_menu_new ();
	amenu = gtk_menu_new ();
/*
	add_to_menu("Copy", GTK_SIGNAL_FUNC(menu_selected),
		    (gpointer)"Copy", menu);
	add_to_menu("Paste", GTK_SIGNAL_FUNC(menu_selected),
		    (gpointer)"Paste", menu);
	add_to_menu("", NULL,NULL, menu);
*/
 	add_to_menu("Rotate Clockwise", 
 		    GTK_SIGNAL_FUNC(manip_pic),
 		    (gpointer)"r", amenu);
 	add_to_menu("Rotate Counter-Clockwise", 
 		    GTK_SIGNAL_FUNC(manip_pic), 
 		    (gpointer)"l", amenu);
 	add_to_menu("Flip Horizontal", 
 		    GTK_SIGNAL_FUNC(manip_pic), 
 		    (gpointer)"h", amenu);
 	add_to_menu("Flip Vertical", 
 		    GTK_SIGNAL_FUNC(manip_pic),
 		    (gpointer)"v", amenu);
 	add_to_menu("Resize", 
 		    GTK_SIGNAL_FUNC(resize_dialog),
 		    (gpointer)"Resize", amenu);
	root_menu = gtk_menu_item_new_with_label("Manipulate");
	gtk_widget_show(root_menu);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM(root_menu), amenu);
	gtk_menu_append(GTK_MENU(menu), root_menu);

	add_to_menu("", NULL,NULL, menu);
	add_to_menu_acc("Select All", GTK_SIGNAL_FUNC(select_all),
		    (gpointer)"Select all", menu,'a',GDK_CONTROL_MASK);
	add_to_menu("Select Inverse", GTK_SIGNAL_FUNC(select_inverse),
		    (gpointer)"Select Inverse", menu);
	add_to_menu_acc("Select None", GTK_SIGNAL_FUNC(select_none),
		    (gpointer)"Select none",menu,'n',GDK_CONTROL_MASK);
	root_menu = gtk_menu_item_new_with_label("Edit");
	gtk_widget_show(root_menu);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM(root_menu), menu);	
	gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), root_menu);

	/* Camera Menu --------------------------------------------- */
	menu = gtk_menu_new ();
	amenu = gtk_menu_new ();
	add_to_menu_acc("Thumbnails", GTK_SIGNAL_FUNC(getindex),
		    (gpointer)"Get Index", amenu,'i',GDK_CONTROL_MASK);
	add_to_menu_acc("No Thumbnails", GTK_SIGNAL_FUNC(getindex_empty),
		    (gpointer)"Get Empty Index", amenu,'e',GDK_CONTROL_MASK);
	root_menu = gtk_menu_item_new_with_label("Get Index");
	gtk_widget_show(root_menu);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM(root_menu), amenu);
	gtk_menu_append(GTK_MENU(menu), root_menu);


	add_to_menu_acc("Get Selected Images", GTK_SIGNAL_FUNC(getpics),
		    NULL, menu,'g',0);
	add_to_menu("Delete Selected Images", GTK_SIGNAL_FUNC(del_dialog),
		    NULL, menu);
	add_to_menu("", NULL,NULL, menu);
	add_to_menu("Take Picture", GTK_SIGNAL_FUNC(takepicture_call),
		    NULL, menu);
	add_to_menu("", NULL,NULL, menu);
	add_to_menu("Camera Summary...", GTK_SIGNAL_FUNC(summary_dialog),
		    NULL, menu);
	add_to_menu("", NULL,NULL, menu);
	add_to_menu("Select Model/Port...", GTK_SIGNAL_FUNC(port_dialog),
		    NULL, menu);
	add_to_menu("Configure...", GTK_SIGNAL_FUNC(configure_call),
		    NULL, menu);
	root_menu = gtk_menu_item_new_with_label("Camera");
	gtk_widget_show(root_menu);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM(root_menu), menu);	
	gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), root_menu);

	/* Plugins Menu -------------------------------------------- */
	menu = gtk_menu_new ();
/*
	add_to_menu("Animated GIF", GTK_SIGNAL_FUNC(menu_selected),
		    (gpointer)"Animated Gif", menu);
*/
	add_to_menu("HTML Gallery", GTK_SIGNAL_FUNC(gallery_main),
		    (gpointer)"HTML Gallery", menu);
	add_to_menu("Live Camera!", GTK_SIGNAL_FUNC(live_main),
		    (gpointer)"Live Camera!", menu);
	root_menu = gtk_menu_item_new_with_label("Plugins");
	gtk_menu_item_right_justify(GTK_MENU_ITEM(root_menu));
	gtk_widget_show(root_menu);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM(root_menu), menu);	
	gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), root_menu);

	/* Help Menu ----------------------------------------------- */
	menu = gtk_menu_new ();
	add_to_menu("User's Manual", GTK_SIGNAL_FUNC(usersmanual_dialog),
		    (gpointer)"How do I...", menu);
	add_to_menu("FAQ's", GTK_SIGNAL_FUNC(faq_dialog),
		    (gpointer)"FAQs", menu);
	add_to_menu("Version Info", GTK_SIGNAL_FUNC(version_dialog),
		    (gpointer)"About", menu);
	add_to_menu("", NULL,NULL, menu);
	add_to_menu("About", GTK_SIGNAL_FUNC(about_dialog),
		    (gpointer)"About", menu);
	root_menu = gtk_menu_item_new_with_label("Help");
	gtk_menu_item_right_justify(GTK_MENU_ITEM(root_menu));
	gtk_widget_show(root_menu);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM(root_menu), menu);	
	gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), root_menu);
}
