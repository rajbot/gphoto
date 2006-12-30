#include "main.h"
#include "callbacks.h"

#include "tb_close.xpm"
/*  #include "tb_copy.xpm" */
/*  #include "tb_cut.xpm" */
#include "tb_exit.xpm"
#include "tb_get_index.xpm"
#include "tb_help.xpm"
#include "tb_index.xpm"
#include "tb_index_empty.xpm"
#include "tb_open.xpm"
/*  #include "tb_paste.xpm" */
#include "tb_preferences.xpm"
#include "tb_print.xpm"
#include "tb_save.xpm"
#include "tb_save_as.xpm"
#include "tb_snapshot.xpm"
#include "tb_trash.xpm"

#include "rotc.xpm"		/* Image Manipulation icons */
#include "rotcc.xpm"
#include "fliph.xpm"
#include "flipv.xpm"
#include "resize.xpm"

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

  add_to_toolbar(mainWin, "Open Image", tb_open_xpm,
                 GTK_SIGNAL_FUNC(filedialog), "o", box, 1);
  add_to_toolbar(mainWin, "Save Current Image", tb_save_xpm,
                 GTK_SIGNAL_FUNC(filedialog), "s", box, 1);
  add_to_toolbar(mainWin, "Batch Save", tb_save_as_xpm,
                 GTK_SIGNAL_FUNC(batch_save_dialog), NULL, box, 1);
  add_to_toolbar(mainWin, "Print", tb_print_xpm,
                 GTK_SIGNAL_FUNC(print_pic), NULL, box, 1);
  add_to_toolbar(mainWin, NULL, NULL, NULL, NULL, box, 1);
  add_to_toolbar(mainWin, "Get Thumbnail Index", tb_get_index_xpm,
                 GTK_SIGNAL_FUNC(getindex), NULL, box, 1);
  add_to_toolbar(mainWin, "Get Empty Index", tb_index_empty_xpm,
                 GTK_SIGNAL_FUNC(getindex_empty), NULL, box, 1);
  add_to_toolbar(mainWin, "Get Selected Images", tb_index_xpm,  
                 GTK_SIGNAL_FUNC(getpics), NULL, box, 1);
  add_to_toolbar(mainWin, "Take picture", tb_snapshot_xpm, 
                 GTK_SIGNAL_FUNC(takepicture_call), NULL, box, 1);
  add_to_toolbar(mainWin, NULL, NULL, NULL, NULL, box, 1);
  add_to_toolbar(mainWin, "Close Downloaded Image", tb_trash_xpm,
                 GTK_SIGNAL_FUNC(closepic), "c", box, 1);  
  add_to_toolbar(mainWin, "Delete Selected Images", tb_close_xpm, 
                 GTK_SIGNAL_FUNC(del_dialog), NULL, box, 1);
/*    add_to_toolbar(mainWin, NULL, NULL, NULL, NULL, box, 1); */
/*    add_to_toolbar(mainWin, "Cut", tb_cut_xpm,  */
/*                   GTK_SIGNAL_FUNC(menu_selected), "Cut", box, 1); */
/*    add_to_toolbar(mainWin, "Copy", tb_copy_xpm, */
/*                   GTK_SIGNAL_FUNC(menu_selected), "Copy", box, 1); */
/*    add_to_toolbar(mainWin, "Paste", tb_paste_xpm,   */
/*                   GTK_SIGNAL_FUNC(menu_selected), "Paste", box, 1); */
  add_to_toolbar(mainWin, NULL, NULL, NULL, NULL, box, 1);
  add_to_toolbar(mainWin, "Camera Configuration", tb_preferences_xpm, 
		 GTK_SIGNAL_FUNC(configure_call), NULL, box, 1);
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
  add_to_toolbar(mainWin, "Help", tb_help_xpm, 
                 GTK_SIGNAL_FUNC(usersmanual_dialog), NULL, box, 1);
  add_to_toolbar(mainWin, NULL, NULL, NULL, NULL, box, 1);
  add_to_toolbar(mainWin, "Exit GNU Photo", tb_exit_xpm,
                 GTK_SIGNAL_FUNC(delete_event), NULL, box, 1);
}
