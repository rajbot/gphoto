#include "main.h"
#include "gphoto.h"
#include "callbacks.h"
#include "gallery.h"
#include "live.h"

/* The toolbar xpm icons */

/*  #include "batch_save.xpm" */
#include "close_image.xpm"
#include "colors.xpm"
#include "configure.xpm"
#include "delete_images.xpm"
#include "exit.xpm"
#include "fliph.xpm"
#include "flipv.xpm"
#include "get_index.xpm"
#include "get_index_empty.xpm"
#include "get_selected_images.xpm"
#include "help.xpm"
#include "left_arrow.xpm"
#include "mail_image.xpm"
#include "open_image.xpm"
#include "print_image.xpm"
#include "resize.xpm"
#include "right_arrow.xpm"
#include "rotc.xpm"
#include "rotcc.xpm"
#include "save_current_image.xpm"
#include "stop.xpm"

/* #include "web_browse.xpm"   */
/* #include "take_picture.xpm" */
/* GtkWidget *browse_button = NULL; */

GtkWidget *stop_button = NULL;

GtkWidget *add_to_toolbar (GtkWidget *mainWin, gchar *tooltipText, 
		     gchar ** xpmIcon, GtkSignalFunc f, gpointer data,
		     GtkWidget *box, int Beginning) {
  
  GtkWidget *button, *gpixmap;
  GdkPixmap *pixmap;
  GdkBitmap *bitmap;
  GtkTooltips *tooltip;
  GtkStyle *style;
  
  if (f == NULL)
    button = gtk_label_new("     ");
  else {
    button = gtk_button_new();
    tooltip = gtk_tooltips_new();
    gtk_tooltips_set_tip(tooltip,button,tooltipText, NULL);
    gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
			      f, data);
    style = gtk_widget_get_style(mainWin);
    pixmap = gdk_pixmap_create_from_xpm_d(mainWin->window,&bitmap,
					  &style->bg[GTK_STATE_NORMAL],
					  xpmIcon);
    gpixmap = gtk_pixmap_new(pixmap,bitmap);
    gtk_widget_show(gpixmap);
    gtk_container_add(GTK_CONTAINER(button), gpixmap);
  }
  gtk_widget_show(button);
  if (Beginning)
    gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);
  else
    gtk_box_pack_end(GTK_BOX(box), button, FALSE, FALSE, 0);
  return (button);
}

void deactivate_button (GtkWidget *cur_button) {
	gtk_widget_set_sensitive(GTK_WIDGET(cur_button), FALSE);
}

void activate_button (GtkWidget *cur_button) {
	gtk_widget_set_sensitive(GTK_WIDGET(cur_button), TRUE);
}

/*  void deactivate_stop_button() { */
/*  	gtk_widget_set_sensitive(GTK_WIDGET(stop_button), FALSE); */
/*  } */

/*  void activate_stop_button() { */
/*  	gtk_widget_set_sensitive(GTK_WIDGET(stop_button), TRUE); */
/*   } */

void create_toolbar (GtkWidget *box, GtkWidget *mainWin) {

  add_to_toolbar(mainWin, N_("Open Image"), open_image_xpm,
                 GTK_SIGNAL_FUNC(open_dialog), NULL, box, 1);
  add_to_toolbar(mainWin, N_("Save Opened Image(s)"), save_current_image_xpm,
                 GTK_SIGNAL_FUNC(save_dialog), NULL, box, 1);
  add_to_toolbar(mainWin, N_("Print Image"), print_image_xpm,
                 GTK_SIGNAL_FUNC(print_pic), NULL, box, 1);
  add_to_toolbar(mainWin, N_("Close Image"), delete_images_xpm,
                 GTK_SIGNAL_FUNC(closepic), "c", box, 1);  
  add_to_toolbar(mainWin, NULL, NULL, NULL, NULL, box, 1);
  add_to_toolbar(mainWin, "Previous page", left_arrow_xpm,
		 GTK_SIGNAL_FUNC(prev_page), "i", box, 1);
  add_to_toolbar(mainWin, "Next page", right_arrow_xpm,
		 GTK_SIGNAL_FUNC(next_page), "i", box, 1);
  add_to_toolbar(mainWin, NULL, NULL, NULL, NULL, box, 1);
  add_to_toolbar(mainWin, "Download Thumbnail Index", get_index_xpm,
                 GTK_SIGNAL_FUNC(getindex), NULL, box, 1);
  add_to_toolbar(mainWin, "Download Empty Index", get_index_empty_xpm,
                 GTK_SIGNAL_FUNC(getindex_empty), NULL, box, 1);
  add_to_toolbar(mainWin, "Download Selected Images", get_selected_images_xpm,
		 GTK_SIGNAL_FUNC(getpics), "i", box, 1);
  add_to_toolbar(mainWin, "Delete Selected Images", close_image_xpm, 
                 GTK_SIGNAL_FUNC(del_dialog), NULL, box, 1);
  add_to_toolbar(mainWin, NULL, NULL, NULL, NULL, box, 1);
  stop_button = add_to_toolbar(mainWin, "Halt Download", stop_xpm,
                 GTK_SIGNAL_FUNC(halt_action), NULL, box, 1);
  deactivate_button(stop_button);
  add_to_toolbar(mainWin, NULL, NULL, NULL, NULL, box, 1);
  add_to_toolbar(mainWin, N_("Rotate Clockwise"), rotc_xpm,
		 GTK_SIGNAL_FUNC(manip_pic), "r", box, 1);
  add_to_toolbar(mainWin, N_("Rotate Counter-Clockwise"), rotcc_xpm,
		 GTK_SIGNAL_FUNC(manip_pic), "l", box, 1);
  add_to_toolbar(mainWin, N_("Flip Horizontal"), fliph_xpm,
		 GTK_SIGNAL_FUNC(manip_pic), "h", box, 1);
  add_to_toolbar(mainWin, N_("Flip Vertical"), flipv_xpm,
		 GTK_SIGNAL_FUNC(manip_pic), "v", box, 1);
  add_to_toolbar(mainWin, N_("Resize"), resize_xpm,
		 GTK_SIGNAL_FUNC(resize_dialog),
		 N_("Resize"), box, 1);
  add_to_toolbar(mainWin, N_("Colors"), colors_xpm,
		 GTK_SIGNAL_FUNC(color_dialog),
		 N_("Colors"), box, 1);
  add_to_toolbar(mainWin, NULL, NULL, NULL, NULL, box, 1);

/*
  browse_button = add_to_toolbar(mainWin, N_("HTML Gallery"), web_browse_xpm,
		 GTK_SIGNAL_FUNC(gallery_main), NULL, box, 1);
  add_to_toolbar(mainWin, N_("Live Camera!"), take_picture_xpm,
                 GTK_SIGNAL_FUNC(live_main), NULL, box, 1);
  add_to_toolbar(mainWin, NULL, NULL, NULL, NULL, box, 1);
*/

  add_to_toolbar(mainWin, N_("Camera Configuration"), configure_xpm, 
		 GTK_SIGNAL_FUNC(configure_call), NULL, box, 1);
  add_to_toolbar(mainWin, NULL, NULL, NULL, NULL, box, 1);
  add_to_toolbar(mainWin, N_("Help"), help_xpm, 
                 GTK_SIGNAL_FUNC(usersmanual_dialog), NULL, box, 1);
  add_to_toolbar(mainWin, N_("Exit gPhoto"), exit_xpm,
                 GTK_SIGNAL_FUNC(delete_event), NULL, box, 1);
}
