/* Konica Q-M100 Camera Functions */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <gdk_imlib.h>
#include <gdk/gdk.h>

#include "qm100.h"
#include "../src/gphoto.h"
#include "konica_qm100.h"
#if defined(FULL_CONFIG)
#include "configDialog.h"
#endif
int konica_picCounter;
void message_window(char *title, char *message, GtkJustification  jtype );

int konica_qm100_initialize()
{
   char *fname;

   qm100_setTrace();
   qm100_setTransmitSpeed();
   konica_picCounter = 0;
   qm100_recovery = 1;
   return(1);
}

void konica_show_camera_error(void)
{
   message_window("Camera failure", qm100_errmsg, GTK_JUSTIFY_FILL);
}

int konica_qm100_number_of_pictures ()
{
  qm100_packet_block packet;
  char cmd[]=QM100_GETSTATUS;
  int serialdev;
  
  if (setjmp(qm100_jmpbuf))
     {
     konica_show_camera_error();
     return 0;
     }
  serialdev = qm100_open(serial_port);
  qm100_transmit(serialdev, cmd, sizeof(cmd), &packet, "Get Status");
  qm100_close(serialdev);
  return PICTURE_COUNT;
}

int konica_qm100_take_picture()
{
  int serialdev;
  qm100_packet_block packet;
  char cmd[]=QM100_GETSTATUS;
  
  if (setjmp(qm100_jmpbuf))
     {
     konica_show_camera_error();
     return 0;
     }
  serialdev = qm100_open(serial_port);
  qm100_takePic(serialdev);
  qm100_transmit(serialdev, cmd, sizeof(cmd), &packet, "Get Status");
  qm100_close(serialdev);
  return PICTURE_COUNT;
}

struct Image *konica_qm100_get_picture (int picNum, int thumbnail)
{
  int serialdev;
  int pid;
  char tempName[1024], rm[1024];
  FILE *jpgfile;
  int jpgfile_size;
  struct Image *im;
  pid = getpid();
  
  if (setjmp(qm100_jmpbuf))
     {
     konica_show_camera_error();
     return NULL;
     }
  serialdev = qm100_open(serial_port);
  konica_picCounter++;
  sprintf(tempName, "%s/gphoto-konica-%i.jpg", gphotoDir, pid,
          konica_picCounter);
  picNum = qm100_getRealPicNum(serialdev, picNum);
  if (thumbnail) 
     qm100_saveThumb(serialdev, tempName, picNum, NULL);
  else 
     qm100_savePic(serialdev, tempName, picNum, NULL);
  qm100_close(serialdev);

  jpgfile = fopen(tempName, "r");
  fseek(jpgfile, 0, SEEK_END);
  jpgfile_size = ftell(jpgfile);
  rewind(jpgfile);
  im = (struct Image*)malloc(sizeof(struct Image));
  im->image = (char *)malloc(sizeof(char)*jpgfile_size);
  fread(im->image, (size_t)sizeof(char), (size_t)jpgfile_size, jpgfile);
  fclose(jpgfile);
  if (thumbnail)
     strcpy(im->image_type, "tif");
  else
     strcpy(im->image_type, "jpg");
  im->image_size = jpgfile_size;
  im->image_info_size = 0;
  remove(tempName);
  return (im);
}

int konica_qm100_delete_picture (int picNum)
{
  int serialdev;
  if (setjmp(qm100_jmpbuf))
     {
     konica_show_camera_error();
     return 0;
     }
  serialdev = qm100_open(serial_port);
  picNum = qm100_getRealPicNum(serialdev, picNum);
  qm100_erasePic(serialdev, picNum);
  qm100_close(serialdev);
  return (1);
}

int konica_qm100_formatCF()
{
  int serialdev;
  if (setjmp(qm100_jmpbuf))
     {
     konica_show_camera_error();
     return 0;
     }
  serialdev = qm100_open(serial_port);
  qm100_formatCF(serialdev);
  qm100_close(serialdev);
  return (1);
}

struct Image *konica_qm100_get_preview () {
  return(0);
}

int konica_qm100_configure ()
{
  GtkWidget *dialog;
#if defined(FULL_CONFIG)
  dialog = qm100_createConfigDlg();
#else
  GtkWidget *button;
  GtkWidget *buttonbox;
  /* Set the Window Dialog up */
  dialog = gtk_window_new(GTK_WINDOW_DIALOG);
  gtk_window_set_title(GTK_WINDOW(dialog), "Camera Configuration");
  gtk_window_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);

  /* Set the buttonbox up */
  buttonbox = gtk_hbutton_box_new();
  gtk_container_border_width(GTK_CONTAINER(buttonbox), 10);

  /* One nice button which calls formatCf please... */
  button = gtk_button_new_with_label ( "Format CF Card" );
  gtk_signal_connect ( GTK_OBJECT(button), "clicked",
		       GTK_SIGNAL_FUNC(konica_qm100_formatCF),
		       NULL);
  gtk_signal_connect_object ( GTK_OBJECT(button), "clicked",
			      GTK_SIGNAL_FUNC(gtk_widget_destroy),
			      GTK_OBJECT(dialog) );
  gtk_container_add ( GTK_CONTAINER(buttonbox), GTK_WIDGET(button));
  gtk_widget_show ( button );

  /* Another to kill this whole window if you will Sir */
  button = gtk_button_new_with_label ( " Cancel " );
  gtk_signal_connect_object ( GTK_OBJECT(button), "clicked",
			      GTK_SIGNAL_FUNC(gtk_widget_destroy),
			      GTK_OBJECT(dialog) );
  gtk_container_add ( GTK_CONTAINER(buttonbox), GTK_WIDGET(button));
  gtk_widget_show ( button );

  /* Stick the buttonbox to the window and show it all */
  gtk_container_add ( GTK_CONTAINER(dialog), GTK_WIDGET(buttonbox));
  gtk_widget_show( buttonbox );
#endif
  gtk_widget_show ( dialog );
  
  return(1);
}

char *konica_qm100_summary()
{
  char summary_string[500];
  char *summary;
  char cmd[]=QM100_GETSTATUS;
  int serialdev;
  qm100_packet_block packet;
  
  update_progress(0);
  if (setjmp(qm100_jmpbuf))
     return qm100_errmsg;
  serialdev = qm100_open(serial_port);
  qm100_transmit(serialdev, cmd, sizeof(cmd), &packet, "Get Status");
  qm100_close(serialdev);
  update_progress(1);

  sprintf(summary_string, 
          "This camera is a Konica QM100 or Hewlett Packard C20/C30\n"
          "It has taken %u pictures and currently contains %d picture(s)\n"
          "The time according to the Camera is %d:%d:%d %d/%d/%d",
          COUNTER, PICTURE_COUNT, TIME_HOUR,TIME_MIN,TIME_SEC, TIME_DAY, TIME_MON, TIME_YEAR);
  summary = (char *)malloc(sizeof(char)*strlen(summary_string)+32);
  strcpy(summary, summary_string);
  return (summary);
}

char *konica_qm100_description()
{
  return("Konica Q-M100 Digital Camera gPhoto Plugin (C) 1999 Phill Hugo / Jesper Skov Works for Q-M100 / HP C20 / HP C30");
}

/* Declare the camera function pointers */

struct _Camera konica_qm100 = {konica_qm100_initialize,
			       konica_qm100_get_picture,
			       konica_qm100_get_preview,
			       konica_qm100_delete_picture,
			       konica_qm100_take_picture,
			       konica_qm100_number_of_pictures,
			       konica_qm100_configure,
			       konica_qm100_summary,
			       konica_qm100_description};
