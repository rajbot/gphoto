#include "main.h"
#include "gphoto.h"
#include "callbacks.h"

/* The toolbar xpm icons */

/* #include "batch_save.xpm" */
#include "close_image.xpm"
#include "configure.xpm"
#include "delete_images.xpm"
#include "exit.xpm"
#include "fliph.xpm"
#include "flipv.xpm"
#include "get_index.xpm"
#include "get_index_empty.xpm"
#include "get_selected_images.xpm"
#include "help.xpm"
#include "open_image.xpm"
#include "print_image.xpm"
#include "resize.xpm"
#include "colors.xpm"
#include "rotc.xpm"
#include "rotcc.xpm"
#include "save_current_image.xpm"
#include "take_picture.xpm"

void add_to_toolbar (GtkWidget *mainWin, gchar *tooltipText, 
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
			      f, (gpointer)data);
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
}

void create_toolbar (GtkWidget *box, GtkWidget *mainWin) {

  add_to_toolbar(mainWin, "Open Image", open_image_xpm,
                 GTK_SIGNAL_FUNC(filedialog), "o", box, 1);
  add_to_toolbar(mainWin, "Save Current Image", save_current_image_xpm,
                 GTK_SIGNAL_FUNC(filedialog), "s", box, 1);
/*  add_to_toolbar(mainWin, "Batch Save", batch_save_xpm,
                 GTK_SIGNAL_FUNC(batch_save_dialog), NULL, box, 1); */
  add_to_toolbar(mainWin, "Print Image", print_image_xpm,
                 GTK_SIGNAL_FUNC(print_pic), NULL, box, 1);
  add_to_toolbar(mainWin, "Close Image", delete_images_xpm,
                 GTK_SIGNAL_FUNC(closepic), "c", box, 1);  
  add_to_toolbar(mainWin, NULL, NULL, NULL, NULL, box, 1);
  add_to_toolbar(mainWin, "Get Thumbnail Index", get_index_xpm,
                 GTK_SIGNAL_FUNC(getindex), NULL, box, 1);
  add_to_toolbar(mainWin, "Get Empty Index", get_index_empty_xpm,
                 GTK_SIGNAL_FUNC(getindex_empty), NULL, box, 1);
  add_to_toolbar(mainWin, "Get Selected Images", get_selected_images_xpm,  
                 GTK_SIGNAL_FUNC(getpics), NULL, box, 1);
  add_to_toolbar(mainWin, "Take picture", take_picture_xpm,
                 GTK_SIGNAL_FUNC(takepicture_call), NULL, box, 1);
  add_to_toolbar(mainWin, "Delete Selected Images", close_image_xpm, 
                 GTK_SIGNAL_FUNC(del_dialog), NULL, box, 1);
/*    add_to_toolbar(mainWin, NULL, NULL, NULL, NULL, box, 1); */
/*    add_to_toolbar(mainWin, "Cut", tb_cut_xpm,  */
/*                   GTK_SIGNAL_FUNC(menu_selected), "Cut", box, 1); */
/*    add_to_toolbar(mainWin, "Copy", tb_copy_xpm, */
/*                   GTK_SIGNAL_FUNC(menu_selected), "Copy", box, 1); */
/*    add_to_toolbar(mainWin, "Paste", tb_paste_xpm,   */
/*                   GTK_SIGNAL_FUNC(menu_selected), "Paste", box, 1); */
  add_to_toolbar(mainWin, NULL, NULL, NULL, NULL, box, 1);
  add_to_toolbar(mainWin, "Rotate Clockwise", rotc_xpm,
		 GTK_SIGNAL_FUNC(manip_pic), "r", box, 1);
  add_to_toolbar(mainWin, "Rotate Counter-Clockwise", rotcc_xpm,
		 GTK_SIGNAL_FUNC(manip_pic), "l", box, 1);
  add_to_toolbar(mainWin, "Flip Horizontal", fliph_xpm,
		 GTK_SIGNAL_FUNC(manip_pic), "h", box, 1);
  add_to_toolbar(mainWin, "Flip Vertical", flipv_xpm,
		 GTK_SIGNAL_FUNC(manip_pic), "v", box, 1);
  add_to_toolbar(mainWin, "Resize", resize_xpm,
		 GTK_SIGNAL_FUNC(resize_dialog),
		 "Resize", box, 1);
  add_to_toolbar(mainWin, "Colors", colors_xpm,
		 GTK_SIGNAL_FUNC(color_dialog),
		 "Colors", box, 1);
  add_to_toolbar(mainWin, NULL, NULL, NULL, NULL, box, 1);
  add_to_toolbar(mainWin, "Camera Configuration", configure_xpm, 
		 GTK_SIGNAL_FUNC(configure_call), NULL, box, 1);
  add_to_toolbar(mainWin, NULL, NULL, NULL, NULL, box, 1);
  add_to_toolbar(mainWin, "Help", help_xpm, 
                 GTK_SIGNAL_FUNC(usersmanual_dialog), NULL, box, 1);
  add_to_toolbar(mainWin, "Exit GNU Photo", exit_xpm,
                 GTK_SIGNAL_FUNC(delete_event), NULL, box, 1);
}
