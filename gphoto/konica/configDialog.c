#include "qm100.h"
#include <sys/stat.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "configDialog.h"

GtkWidget *qm100_createConfigDlg ()
{
  GtkWidget *qm100ConfigDlg;
  GtkWidget *dialog_vbox1;
  GtkWidget *notebook1;
  GtkWidget *vbox2;
  GtkWidget *label4;
  GSList *Size_group = NULL;
  GtkWidget *normalSize;
  GtkWidget *doubleSize;
  GtkWidget *vbox1;
  GSList *Speed_group = NULL;
  GtkWidget *spd_9600;
  GtkWidget *spd_19200;
  GtkWidget *spd_38400;
  GtkWidget *spd_57600;
  GtkWidget *spd_115200;
  GtkWidget *vbox3;
  GtkWidget *debugToggle;
  GtkWidget *Photos;
  GtkWidget *PortSpeed;
  GtkWidget *debug;
  GtkWidget *dialog_action_area1;
  GtkWidget *hbuttonbox1;
  GtkWidget *okBtn;
  GtkWidget *cancelBtn;

  qm100ConfigDlg = gtk_dialog_new ();
  gtk_widget_set_name (qm100ConfigDlg, "qm100ConfigDlg");
  gtk_object_set_data (GTK_OBJECT (qm100ConfigDlg), "qm100ConfigDlg", qm100ConfigDlg);
  GTK_WIDGET_SET_FLAGS (qm100ConfigDlg, GTK_CAN_DEFAULT);
  gtk_widget_grab_focus (qm100ConfigDlg);
  gtk_widget_grab_default (qm100ConfigDlg);
  gtk_widget_set_extension_events (qm100ConfigDlg, GDK_EXTENSION_EVENTS_ALL);
  gtk_window_set_title (GTK_WINDOW (qm100ConfigDlg), "Configure Konica/HP Camera");
  gtk_window_set_policy (GTK_WINDOW (qm100ConfigDlg), TRUE, TRUE, FALSE);

  dialog_vbox1 = GTK_DIALOG (qm100ConfigDlg)->vbox;
  gtk_widget_set_name (dialog_vbox1, "dialog_vbox1");
  gtk_object_set_data (GTK_OBJECT (qm100ConfigDlg), "dialog_vbox1", dialog_vbox1);
  gtk_widget_show (dialog_vbox1);

  notebook1 = gtk_notebook_new ();
  gtk_widget_set_name (notebook1, "notebook1");
  gtk_object_set_data (GTK_OBJECT (qm100ConfigDlg), "notebook1", notebook1);
  gtk_widget_show (notebook1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), notebook1, TRUE, TRUE, 0);

  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (vbox2, "vbox2");
  gtk_object_set_data (GTK_OBJECT (qm100ConfigDlg), "vbox2", vbox2);
  gtk_widget_show (vbox2);
  gtk_container_add (GTK_CONTAINER (notebook1), vbox2);

  label4 = gtk_label_new ("Size to use for low resolution photos:");
  gtk_widget_set_name (label4, "label4");
  gtk_object_set_data (GTK_OBJECT (qm100ConfigDlg), "label4", label4);
  gtk_widget_show (label4);
  gtk_box_pack_start (GTK_BOX (vbox2), label4, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label4), GTK_JUSTIFY_LEFT);

  normalSize = gtk_radio_button_new_with_label (Size_group, "320 x 240");
  Size_group = gtk_radio_button_group (GTK_RADIO_BUTTON (normalSize));
  gtk_widget_set_name (normalSize, "normalSize");
  gtk_object_set_data (GTK_OBJECT (qm100ConfigDlg), "normalSize", normalSize);
  gtk_widget_show (normalSize);
  gtk_box_pack_start (GTK_BOX (vbox2), normalSize, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (normalSize), "pressed",
                      GTK_SIGNAL_FUNC (setSize),
                      0);

  doubleSize = gtk_radio_button_new_with_label (Size_group, "640 x 480");
  Size_group = gtk_radio_button_group (GTK_RADIO_BUTTON (doubleSize));
  gtk_widget_set_name (doubleSize, "doubleSize");
  gtk_object_set_data (GTK_OBJECT (qm100ConfigDlg), "doubleSize", doubleSize);
  gtk_widget_show (doubleSize);
  gtk_box_pack_start (GTK_BOX (vbox2), doubleSize, FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (doubleSize), "pressed",
                      GTK_SIGNAL_FUNC (setSize),
                      (gpointer) 1);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (vbox1, "vbox1");
  gtk_object_set_data (GTK_OBJECT (qm100ConfigDlg), "vbox1", vbox1);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (notebook1), vbox1);

  spd_9600 = gtk_radio_button_new_with_label (Speed_group, "9600 (Default)");
  Speed_group = gtk_radio_button_group (GTK_RADIO_BUTTON (spd_9600));
  gtk_widget_set_name (spd_9600, "spd_9600");
  gtk_object_set_data (GTK_OBJECT (qm100ConfigDlg), "spd_9600", spd_9600);
  gtk_widget_show (spd_9600);
  gtk_box_pack_start (GTK_BOX (vbox1), spd_9600, FALSE, FALSE, 0);
  gtk_signal_connect_after (GTK_OBJECT (spd_9600), "pressed",
                            GTK_SIGNAL_FUNC (on_spd_pressed),
                            (gpointer) B9600);

  spd_19200 = gtk_radio_button_new_with_label (Speed_group, "19200");
  Speed_group = gtk_radio_button_group (GTK_RADIO_BUTTON (spd_19200));
  gtk_widget_set_name (spd_19200, "spd_19200");
  gtk_object_set_data (GTK_OBJECT (qm100ConfigDlg), "spd_19200", spd_19200);
  gtk_widget_show (spd_19200);
  gtk_box_pack_start (GTK_BOX (vbox1), spd_19200, FALSE, FALSE, 0);
  gtk_signal_connect_after (GTK_OBJECT (spd_19200), "pressed",
                            GTK_SIGNAL_FUNC (on_spd_pressed),
                            (gpointer) B19200);

  spd_38400 = gtk_radio_button_new_with_label (Speed_group, "38400");
  Speed_group = gtk_radio_button_group (GTK_RADIO_BUTTON (spd_38400));
  gtk_widget_set_name (spd_38400, "spd_38400");
  gtk_object_set_data (GTK_OBJECT (qm100ConfigDlg), "spd_38400", spd_38400);
  gtk_widget_show (spd_38400);
  gtk_box_pack_start (GTK_BOX (vbox1), spd_38400, FALSE, FALSE, 0);
  gtk_signal_connect_after (GTK_OBJECT (spd_38400), "pressed",
                            GTK_SIGNAL_FUNC (on_spd_pressed),
                            (gpointer) B38400);

  spd_57600 = gtk_radio_button_new_with_label (Speed_group, "57600");
  Speed_group = gtk_radio_button_group (GTK_RADIO_BUTTON (spd_57600));
  gtk_widget_set_name (spd_57600, "spd_57600");
  gtk_object_set_data (GTK_OBJECT (qm100ConfigDlg), "spd_57600", spd_57600);
  gtk_widget_show (spd_57600);
  gtk_box_pack_start (GTK_BOX (vbox1), spd_57600, FALSE, FALSE, 0);
  gtk_signal_connect_after (GTK_OBJECT (spd_57600), "pressed",
                            GTK_SIGNAL_FUNC (on_spd_pressed),
                            (gpointer) B57600);

  spd_115200 = gtk_radio_button_new_with_label (Speed_group, "115200");
  Speed_group = gtk_radio_button_group (GTK_RADIO_BUTTON (spd_115200));
  gtk_widget_set_name (spd_115200, "spd_115200");
  gtk_object_set_data (GTK_OBJECT (qm100ConfigDlg), "spd_115200", spd_115200);
  gtk_widget_show (spd_115200);
  gtk_box_pack_start (GTK_BOX (vbox1), spd_115200, FALSE, FALSE, 0);
  gtk_signal_connect_after (GTK_OBJECT (spd_115200), "pressed",
                            GTK_SIGNAL_FUNC (on_spd_pressed),
                            (gpointer) B115200);

  vbox3 = gtk_vbox_new (FALSE, 0);
  gtk_widget_set_name (vbox3, "vbox3");
  gtk_object_set_data (GTK_OBJECT (qm100ConfigDlg), "vbox3", vbox3);
  gtk_widget_show (vbox3);
  gtk_container_add (GTK_CONTAINER (notebook1), vbox3);

  debugToggle = gtk_check_button_new_with_label ("Turn on debugging");
  gtk_widget_set_name (debugToggle, "debugToggle");
  gtk_object_set_data (GTK_OBJECT (qm100ConfigDlg), "debugToggle", debugToggle);
  gtk_widget_show (debugToggle);
  gtk_box_pack_start (GTK_BOX (vbox3), debugToggle, FALSE, FALSE, 0);

  Photos = gtk_label_new ("Photos");
  gtk_widget_set_name (Photos, "Photos");
  gtk_object_set_data (GTK_OBJECT (qm100ConfigDlg), "Photos", Photos);
  gtk_widget_show (Photos);
  set_notebook_tab (notebook1, 0, Photos);
  gtk_label_set_justify (GTK_LABEL (Photos), GTK_JUSTIFY_LEFT);

  PortSpeed = gtk_label_new ("Port Speed");
  gtk_widget_set_name (PortSpeed, "PortSpeed");
  gtk_object_set_data (GTK_OBJECT (qm100ConfigDlg), "PortSpeed", PortSpeed);
  gtk_widget_show (PortSpeed);
  set_notebook_tab (notebook1, 1, PortSpeed);

  debug = gtk_label_new ("Debug");
  gtk_widget_set_name (debug, "debug");
  gtk_object_set_data (GTK_OBJECT (qm100ConfigDlg), "debug", debug);
  gtk_widget_show (debug);
  set_notebook_tab (notebook1, 2, debug);

  dialog_action_area1 = GTK_DIALOG (qm100ConfigDlg)->action_area;
  gtk_widget_set_name (dialog_action_area1, "dialog_action_area1");
  gtk_object_set_data (GTK_OBJECT (qm100ConfigDlg), "dialog_action_area1", dialog_action_area1);
  gtk_widget_show (dialog_action_area1);
  gtk_container_border_width (GTK_CONTAINER (dialog_action_area1), 10);

  hbuttonbox1 = gtk_hbutton_box_new ();
  gtk_widget_set_name (hbuttonbox1, "hbuttonbox1");
  gtk_object_set_data (GTK_OBJECT (qm100ConfigDlg), "hbuttonbox1", hbuttonbox1);
  gtk_widget_show (hbuttonbox1);
  gtk_box_pack_start (GTK_BOX (dialog_action_area1), hbuttonbox1, TRUE, TRUE, 0);

  okBtn = gtk_button_new_with_label ("Ok");
  gtk_widget_set_name (okBtn, "okBtn");
  gtk_object_set_data (GTK_OBJECT (qm100ConfigDlg), "okBtn", okBtn);
  gtk_widget_show (okBtn);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), okBtn);
  gtk_signal_connect (GTK_OBJECT (okBtn), "clicked",
                      GTK_SIGNAL_FUNC (on_okBtn_clicked),
                      qm100ConfigDlg);
  gtk_signal_connect_object_after (GTK_OBJECT (okBtn), "clicked",
                                   GTK_SIGNAL_FUNC (gtk_widget_destroy),
                                   GTK_OBJECT (qm100ConfigDlg));

  cancelBtn = gtk_button_new_with_label ("Cancel");
  gtk_widget_set_name (cancelBtn, "cancelBtn");
  gtk_object_set_data (GTK_OBJECT (qm100ConfigDlg), "cancelBtn", cancelBtn);
  gtk_widget_show (cancelBtn);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), cancelBtn);
  gtk_signal_connect_object (GTK_OBJECT (cancelBtn), "clicked",
                             GTK_SIGNAL_FUNC (gtk_widget_destroy),
                             GTK_OBJECT (qm100ConfigDlg));

  return qm100ConfigDlg;
}

void on_okBtn_clicked(GtkButton *button, gpointer user_data)
{}

void setSize(GtkButton *button, gpointer user_data)
{}

void on_spd_pressed(GtkButton *button, gpointer user_data)
{}
void cancel_btn_clicked(GtkButton *button, gpointer user_data)
{}
