#include "dc210.h"
#include "config.h"
#include "kodak_dc210.h"

#include "../src/gphoto.h"
#include "../src/util.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <termios.h>
#include <fcntl.h>


/* glib is used for byte swapping and endian checking */
#include <glib.h>
#include <gdk_imlib.h>
#include <gdk/gdk.h>

#include <errno.h>

#ifdef HAVE_SELECT_H
#include <sys/select.h>
#endif /* HAVE_SELECT_H */

#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#    ifdef HAVE_SYS_TIME_H
#    include <sys/time.h>
#    else HAVE_SYS_TIME_H
#    include <time.h>
#    endif /* HAVE_SYS_TIME_H */
#endif /* TIME_WITH_SYS_TIME */

int serialdev;

char *zoomstr[]={"58mm","51mm","41mm","34mm","29mm","Macro"};
char *flashstr[]={"Auto","On","Off","Auto Red Eye","On w/Red Eye"};
char *qualstr[]={"","Best","Better","Good"};

/* printf to stderr */
void eprintf(char *fmt, ...)
{
    char msg[132];
    va_list args;
    va_start(args, fmt);
    vsprintf(msg, fmt, args);
    fprintf(stderr,msg);
    va_end(args);
}

/* Send command, read ack from cam */
int kodak_dc210_send_command ( char command, int arg1, int arg2, int arg3, int arg4)
{
  unsigned char ack;
  int ret=RET_ERROR;
  char packet[9];

  sendpck:
  sprintf(packet,"%c%c%c%c%c%c%c%c",command,0x00,arg1,arg2,arg3,arg4,0x00,0x1A);
  write(serialdev, packet, 8);
  ret=kodak_dc210_read( &ack, 1 );
  if (ret!=RET_OK) {
    eprintf("kodak_dc210_send_command(): camera timeout, resending packet\n");
    goto sendpck;
  }

  switch (ack) {
   case DC_COMMAND_ACK:
    return RET_OK;
   break;
   case DC_EXECUTION_ERROR:
    eprintf("kodak_dc210_send_command(): illegal command\n");
    return RET_ERROR;
   break;
   case DC_COMMAND_NAK:
    eprintf("kodak_dc210_send_command(): wrong mode for operation\n");
    return RET_ERROR;
   break;
   default:
    eprintf("kodak_dc210_send_command(): we shouldn't be here\n");
    return RET_ERROR;
   break;
  }
}

/* Change port speed to transfer images quickly */
void kodak_dc210_set_port_speed ( int speed ) {
  struct termios newt, oldt;
  int arg1,arg2;

  if (tcgetattr(serialdev, &oldt) < 0) {
    eprintf("Cannot get serial parameters.\n");
    exit(1);
  }
  memcpy((char *)&newt,(char *)&oldt, sizeof(struct termios));
  arg1 = speed ? 0x11 : 0x96;
  arg2 = speed ? 0x52 : 0x00;
  kodak_dc210_send_command(DC_SET_SPEED,arg1,arg2,0x00,0x00);
  usleep(200000); /* 200ms sleep after speed change */
  cfsetospeed(&newt, speed ? B115200 : B9600);
  cfsetispeed(&newt, speed ? B115200 : B9600);
  if (tcsetattr(serialdev, TCSANOW, &newt) < 0) {
    eprintf("Cannot set serial parameters.\n");
    exit(1);
  }
}

/* Sent after packet read is complete or after command is done */
int kodak_dc210_command_complete(void) 
{
  int status=DC_BUSY;

  do {
    kodak_dc210_read( (char *)&status,1);
  } while (status == DC_BUSY);
 
  usleep(100000);
  if (status == DC_COMMAND_COMPLETE )
    return RET_OK;
  return RET_ERROR;
}

/* Camera read routine */
int kodak_dc210_read (unsigned char *buf, int nbytes )
{
  int ret;
  int numRead = 0;
  int n;
  int len;
  fd_set readfds;
  struct timeval timeout;

  while (numRead < nbytes) {
    n = serialdev + 1;
    FD_ZERO(&readfds);
    FD_SET(serialdev, &readfds);
    timeout.tv_sec = 6;
    timeout.tv_usec = 500000;
    ret = select(n, &readfds, NULL, NULL, &timeout);

    if (ret <= 0) {
      
      if (ret<0)
	if (errno == EINTR) continue; // while loop
        else perror("select");
      else {
	eprintf("kodak_dc210_read(): timeout\n");
	return RET_TIMEOUT;
      }
      return RET_ERROR;

    } else {
      if (FD_ISSET(serialdev, &readfds)) {
        len = read(serialdev, buf + numRead, nbytes - numRead);
        if (len < 0) 
	  if (errno == EINTR) continue;
	else { 
	  eprintf("kodak_dc210_read(): fail on ready file handle\n");
	  perror("read");
	  return RET_ERROR;
        }
        else numRead += len;
      }
      else return RET_OK;
    }
  } // end while
  return RET_OK;
}

/* Camera write routine */
int kodak_dc210_write(char byte)
{
  write(serialdev, &byte, 1);
  return RET_OK;
}

unsigned char kodak_dc210_checksum(char *packet,int length)
{
  unsigned char chksum = 0;
  int i;
  for (i = 0 ; i < length; i++)
    chksum ^= packet[i];
  return chksum;
}

/* Read packet size length from camera */
int kodak_dc210_read_packet ( char *packet, int length)
{
  unsigned char chk,camchk;
  unsigned char header;
  int retry=0;

  readpck:
  kodak_dc210_read(&header,1);

  if (header==DC_EXECUTION_ERROR) {
    eprintf("kodak_dc210_read_packet(): error in command arguments\n");
    return RET_ERROR;
  }
  kodak_dc210_read(packet,length);
  kodak_dc210_read(&camchk,1);

  chk = kodak_dc210_checksum(packet,length);
  
  if (camchk != chk) {
    retry++;
    // eprintf("kodak_dc210_read_packet(): checksum %d != %d, reading packet again\n",chk,camchk);
    if (retry>5) { 
      kodak_dc210_write(DC_CANCEL);
      error_dialog("Serial Port communication hosed.  Try command again.");
      return RET_ERROR;
    }
    kodak_dc210_write(DC_ILLEGAL_PACKET);
    goto readpck;
  }  
  kodak_dc210_write(DC_CORRECT_PACKET);
  return RET_OK;
}

int kodak_dc210_initialize(void) 
{
    serialdev=kodak_dc210_open_camera(serial_port);
    kodak_dc210_send_command(DC_GET_BATTERY,0x00,0x00,0x00,0x00);
    kodak_dc210_command_complete();
    eprintf("Initialization complete\n");
    return RET_OK;
}

/* Open serial port attached to the camera */
int kodak_dc210_open_camera (const char *devname)
{
  struct termios newt, oldt;

  serialdev = open(devname, O_RDWR|O_NOCTTY|O_NONBLOCK);
  if (serialdev < 0) {
     eprintf("Cannot open %s\n",devname);
     exit(1);
  }
  if (tcgetattr(serialdev, &oldt) < 0) { 
     eprintf("Cannot get serial parameters for %s\n",devname);
     exit(1);
  }
  memcpy((char *)&newt,(char *)&oldt, sizeof(struct termios));
  cfmakeraw(&newt);
  newt.c_cflag &= ~CSTOPB;
  newt.c_cflag &= ~PARENB;
  newt.c_cflag |= CS8;
  newt.c_cflag &= ~PARODD;
  newt.c_cflag |= CRTSCTS;
  newt.c_cc[VMIN] = 0;
  newt.c_cc[VTIME] = 10;
  cfsetospeed(&newt, B9600);
  cfsetispeed(&newt, B9600);
  if (tcsetattr(serialdev, TCSANOW, &newt) < 0) {
    eprintf("Cannot set serial parameters for %s\n",devname);
    exit(1);
  }
  return serialdev;
}

int kodak_dc210_close_camera () 
{
  close(serialdev);
  return RET_OK;
}

int kodak_dc210_number_of_pictures () 
{
  int num = 0;
  struct kodak_dc210_status status;

  kodak_dc210_get_camera_status(&status);
  num = status.pictures_in_camera;
  return num;
}

int kodak_dc210_take_picture() 
{
  int j;
 
  kodak_dc210_send_command(DC_TAKE_PICTURE,0x00,0x00,0x00,0x00);
  update_progress(0.00);
  for (j=0;j<10;j++) {
   usleep(300000);
   update_progress((float)j/10);
  }
  update_progress(1.00);
  kodak_dc210_command_complete();
	 
  return kodak_dc210_number_of_pictures();
}

int kodak_dc210_get_picture_info (int picNum, struct kodak_dc210_picture_info *picinfo)
{
  unsigned char packet[PACKET_STATUS_DATA];

  kodak_dc210_send_command(DC_GET_PICINFO,0x00,picNum-1,0x00,0x00);
  kodak_dc210_read_packet(packet,PACKET_STATUS_DATA);
  memset((char*)picinfo,0,sizeof(struct kodak_dc210_picture_info));

  picinfo->camera_type		= packet[1];
  picinfo->file_type		= packet[2];
  picinfo->resolution		= packet[3];
  picinfo->compression		= packet[4];
  picinfo->picture_number	= packet[6] * 0x100 + packet[7];
  picinfo->picture_size		= packet[8]  * 0x1000000 + 
				  packet[9]  * 0x10000 +
				  packet[10] * 0x100 +
				  packet[11];
  picinfo->picture_time		= packet[12] * 0x1000000 +
				  packet[13] * 0x10000 +
				  packet[14] * 0x100 +
				  packet[15];
  picinfo->flash_used		= packet[16];
  picinfo->flash_mode		= packet[17];
  picinfo->zoom_position	= packet[21];
  picinfo->f_number		= packet[26];
  picinfo->battery		= packet[27];
  picinfo->exposure_time	= packet[28]  * 0x1000000 +
				  packet[29]  * 0x10000 +
				  packet[30] * 0x100 +
				  packet[31];
  strncpy(picinfo->image_name,packet+32,12);
  kodak_dc210_command_complete();
}

int kodak_dc210_get_camera_status(struct kodak_dc210_status *status)
{
  unsigned char packet[PACKET_STATUS_DATA];

  kodak_dc210_send_command(DC_GET_STATUS,0x00,0x00,0x00,0x00);
  kodak_dc210_read_packet(packet,PACKET_STATUS_DATA); 
  memset((char*)status,0,sizeof(struct kodak_dc210_status));

  status->camera_type		= packet[1];
  status->firmware_major	= packet[2];
  status->firmware_minor	= packet[3];
  status->dsp_major		= packet[4];
  status->dsp_minor		= packet[5];
  status->mu_major		= packet[6];
  status->mu_minor		= packet[7];
  status->battery		= packet[8];
  status->acstatus		= packet[9];
  status->camera_time = EPOC + (packet[12] * 0x1000000 + packet[13] * 0x10000 + packet[14] * 0x100 + packet[15]) / 2;
  status->zoom_position		= packet[16];
  status->flash_status		= packet[18];
  status->compression		= packet[19];
  status->flash_mode		= packet[20];
  status->exp_compensation	= packet[21];
  status->resolution		= packet[22];
  status->filetype		= packet[23];
  status->total_pictures	= packet[25] * 0x100 + packet[26];
  status->total_flash		= packet[27] * 0x100 + packet[28];
  status->timer_mode		= packet[29];
  status->memory_card_status	= packet[30];
  status->video_format		= packet[31];
  status->comm_mode		= packet[32];
  status->pictures_in_camera	= packet[56] * 0x100 + packet[57];
  status->remaining_low		= packet[68] * 0x100 + packet[69];
  status->remaining_med		= packet[70] * 0x100 + packet[71];
  status->remaining_high	= packet[72] * 0x100 + packet[73];
  strncpy(status->volume_id,packet+77,11);
  strncpy(status->camera_id,packet+90,32);
  
  kodak_dc210_command_complete();
  return RET_OK;
}

struct Image *kodak_dc210_get_thumbnail (int picNum) 
{
  struct Image *im = NULL;
  int i,j;
  int numRead = 0;
  int success = RET_OK;
  int fileSize = 20736;
  char *picData;
  char *imData;
  char bmpHeader[] = {
        0x42, 0x4d, 0x36, 0x24, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00,
	0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x48, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

  picData = (char *)malloc(fileSize + PACKET_IMAGE_DATA);
  imData = (char *)malloc(fileSize + 54);

  update_progress(0.00);

  kodak_dc210_set_port_speed(SPD_VHI);
  kodak_dc210_send_command(DC_GET_THUMB,0x00,picNum-1,0x01,0x00);
  
  while (success && (numRead < fileSize))
    {
      success=kodak_dc210_read_packet(picData+numRead,PACKET_IMAGE_DATA);
      numRead += PACKET_IMAGE_DATA;

      if (numRead <= fileSize)
	  update_progress((float)numRead / (float)fileSize);
    }

    kodak_dc210_command_complete();
    kodak_dc210_set_port_speed(SPD_LOW);
    update_progress(1.00);

      /* allocate memory for image structure */
      if ( (im = (struct Image *)malloc ( sizeof(struct Image) )) == NULL ) 
      {
	error_dialog("Could not allocate memory for image structure.");
	return ( NULL );
      }
    
      memcpy(imData,bmpHeader, sizeof(bmpHeader));

      /* reverse the thumbnail data */
      /* not only is the data reversed but the image is flipped
       * left to right
       */
      for (i = 0; i < 72; i++) {
         for (j = 0; j < 96; j++) {
            imData[i*96*3+j*3+54] = picData[(71-i)*96*3+j*3+2];
            imData[i*96*3+j*3+54+1] = picData[(71-i)*96*3+j*3+1];
            imData[i*96*3+j*3+54+2] = picData[(71-i)*96*3+j*3];
            }
         }

      strcpy ( im->image_type, "bmp" );
      im->image_info = NULL;
      im->image_info_size = 0;
      im->image_size = fileSize+54;
      im->image = imData;

  free(picData);

  return im;
}

struct Image *kodak_dc210_get_picture (int picNum, int thumbnail) 
{
  struct Image *im = NULL;
  int fileSize;
  int numRead;
  int serialdev;
  struct kodak_dc210_picture_info picInfo;
  char *picData;

  if (thumbnail)
  {
    im = kodak_dc210_get_thumbnail(picNum);
  }
  else
  {
      /* get information (size, name, etc) about the picture */
      kodak_dc210_get_picture_info (picNum, &picInfo);

      kodak_dc210_set_port_speed(SPD_VHI);
      /* send the command to start transfering the picture */
      kodak_dc210_send_command(DC_GET_PICTURE, 0x00, picNum-1, 0x00, 0x00);

      fileSize = picInfo.picture_size;
      picData = (char *)malloc(fileSize + PACKET_IMAGE_DATA);
      numRead = 0;

      update_progress(0.00);
      while (numRead < fileSize)
      {
	kodak_dc210_read_packet(picData+numRead,PACKET_IMAGE_DATA);
	numRead += PACKET_IMAGE_DATA;

	if (numRead <= fileSize)
	  update_progress((float)numRead / (float)fileSize);
      }
      kodak_dc210_command_complete();
      kodak_dc210_set_port_speed(SPD_LOW);
      update_progress(1.00);

      /* allocate memory for image structure */
      if ( (im = (struct Image *)malloc ( sizeof(struct Image) )) == NULL ) 
      {
	error_dialog("Could not allocate memory for image structure.");
	return ( NULL );
      }
      
      strcpy ( im->image_type, "jpg" );
      im->image_info = NULL;
      im->image_info_size = 0;
      im->image_size = fileSize;
      im->image = picData;

    }

  return(im);
}

int kodak_dc210_delete_picture (int picNum) 
{
    kodak_dc210_send_command(DC_ERASE_IMAGE,0x00,picNum-1,0x00,0x00);
    kodak_dc210_command_complete();
    return RET_OK;
}

struct Image *kodak_dc210_get_preview () 
{
  int numPicBefore;
  int numPicAfter;
  struct Image *im = NULL;

  numPicBefore = kodak_dc210_number_of_pictures();

  numPicAfter = kodak_dc210_take_picture();

  /* if a picture was taken then get the picture from the camera and
     then delete it */
  if (numPicBefore + 1 == numPicAfter)
  {
    im = kodak_dc210_get_picture (numPicAfter,0);
    kodak_dc210_delete_picture (numPicAfter);
  }

  return(im);
}

GtkWidget*
create_dc210 (void)
{
  struct kodak_dc210_status status;
  GtkWidget *window;
  GtkWidget *fixed;
  GtkWidget *flash_o;
  GtkWidget *flash_o_menu;
  GtkWidget *glade_menuitem;
  GtkWidget *ok_b;
  GtkWidget *cancel_b;
  GtkWidget *quality_o;
  GtkWidget *quality_o_menu;
  GtkWidget *camera_id;
  GtkWidget *label5;
  GtkWidget *label1;
  GtkWidget *label4;
  GtkWidget *label6;
  GtkWidget *res_o;
  GtkWidget *res_o_menu;
  GtkWidget *label7;
  GtkWidget *zoom_o;
  GtkWidget *zoom_o_menu;

  window = gtk_dialog_new ();
  kodak_dc210_get_camera_status(&status);
  gtk_object_set_data (GTK_OBJECT (window), "window", window);
  gtk_widget_set_usize (window, 177, 177);
  gtk_window_set_title (GTK_WINDOW (window), "DC21x Settings");

  fixed = gtk_fixed_new ();
  gtk_widget_ref (fixed);
  gtk_object_set_data_full (GTK_OBJECT (window), "fixed", fixed,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fixed);
  gtk_container_add (GTK_CONTAINER (window), fixed);

  flash_o = gtk_option_menu_new ();
  gtk_widget_ref (flash_o);
  gtk_object_set_data_full (GTK_OBJECT (window), "flash_o", flash_o,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (flash_o);
  gtk_fixed_put (GTK_FIXED (fixed), flash_o, 8, 64);
  gtk_widget_set_uposition (flash_o, 8, 64);
  gtk_widget_set_usize (flash_o, 64, 24);
  flash_o_menu = gtk_menu_new ();
  glade_menuitem = gtk_menu_item_new_with_label ("Auto");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (flash_o_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("Flash On");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (flash_o_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("Flash Off");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (flash_o_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("Red Eye Auto");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (flash_o_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("Red Eye On");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (flash_o_menu), glade_menuitem);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (flash_o), flash_o_menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (flash_o), status.flash_mode);
  
  ok_b = gtk_button_new_with_label ("Set");
  gtk_widget_ref (ok_b);
  gtk_object_set_data_full (GTK_OBJECT (window), "ok_b", ok_b,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (ok_b);
  gtk_fixed_put (GTK_FIXED (fixed), ok_b, 88, 144);
  gtk_widget_set_uposition (ok_b, 88, 144);
  gtk_widget_set_usize (ok_b, 80, 24);

  cancel_b = gtk_button_new_with_label ("Cancel");
  gtk_widget_ref (cancel_b);
  gtk_object_set_data_full (GTK_OBJECT (window), "cancel_b", cancel_b,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (cancel_b);
  gtk_fixed_put (GTK_FIXED (fixed), cancel_b, 88, 112);
  gtk_widget_set_uposition (cancel_b, 88, 112);
  gtk_widget_set_usize (cancel_b, 80, 28);

  quality_o = gtk_option_menu_new ();
  gtk_widget_ref (quality_o);
  gtk_object_set_data_full (GTK_OBJECT (window), "quality_o", quality_o,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (quality_o);
  gtk_fixed_put (GTK_FIXED (fixed), quality_o, 8, 104);
  gtk_widget_set_uposition (quality_o, 8, 104);
  gtk_widget_set_usize (quality_o, 64, 24);
  quality_o_menu = gtk_menu_new ();
  glade_menuitem = gtk_menu_item_new_with_label ("Best");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (quality_o_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("Better");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (quality_o_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("Good");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (quality_o_menu), glade_menuitem);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (quality_o), quality_o_menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (quality_o), status.compression-1);
  
  camera_id = gtk_entry_new_with_max_length (32);
  gtk_widget_ref (camera_id);
  gtk_object_set_data_full (GTK_OBJECT (window), "camera_id", camera_id,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (camera_id);
  gtk_fixed_put (GTK_FIXED (fixed), camera_id, 8, 24);
  gtk_widget_set_uposition (camera_id, 8, 24);
  gtk_widget_set_usize (camera_id, 158, 20);
  gtk_entry_set_text (GTK_ENTRY (camera_id), status.camera_id);

  label5 = gtk_label_new ("Camera ID");
  gtk_widget_ref (label5);
  gtk_object_set_data_full (GTK_OBJECT (window), "label5", label5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label5);
  gtk_fixed_put (GTK_FIXED (fixed), label5, 8, 8);
  gtk_widget_set_uposition (label5, 8, 8);
  gtk_widget_set_usize (label5, 160, 16);
  gtk_label_set_justify (GTK_LABEL (label5), GTK_JUSTIFY_LEFT);
  gtk_label_set_line_wrap (GTK_LABEL (label5), TRUE);

  label1 = gtk_label_new ("Flash Mode");
  gtk_widget_ref (label1);
  gtk_object_set_data_full (GTK_OBJECT (window), "label1", label1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label1);
  gtk_fixed_put (GTK_FIXED (fixed), label1, 8, 48);
  gtk_widget_set_uposition (label1, 8, 48);
  gtk_widget_set_usize (label1, 96, 16);
  gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);
  gtk_label_set_line_wrap (GTK_LABEL (label1), TRUE);

  label4 = gtk_label_new ("Quality");
  gtk_widget_ref (label4);
  gtk_object_set_data_full (GTK_OBJECT (window), "label4", label4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label4);
  gtk_fixed_put (GTK_FIXED (fixed), label4, 8, 88);
  gtk_widget_set_uposition (label4, 8, 88);
  gtk_widget_set_usize (label4, 40, 16);

  label6 = gtk_label_new ("Resolution");
  gtk_widget_ref (label6);
  gtk_object_set_data_full (GTK_OBJECT (window), "label6", label6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label6);
  gtk_fixed_put (GTK_FIXED (fixed), label6, 8, 128);
  gtk_widget_set_uposition (label6, 8, 128);
  gtk_widget_set_usize (label6, 64, 16);
  gtk_label_set_justify (GTK_LABEL (label6), GTK_JUSTIFY_LEFT);
  gtk_label_set_line_wrap (GTK_LABEL (label6), TRUE);

  res_o = gtk_option_menu_new ();
  gtk_widget_ref (res_o);
  gtk_object_set_data_full (GTK_OBJECT (window), "res_o", res_o,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (res_o);
  gtk_fixed_put (GTK_FIXED (fixed), res_o, 8, 144);
  gtk_widget_set_uposition (res_o, 8, 144);
  gtk_widget_set_usize (res_o, 64, 24);
  res_o_menu = gtk_menu_new ();
  glade_menuitem = gtk_menu_item_new_with_label ("High");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (res_o_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("Low");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (res_o_menu), glade_menuitem);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (res_o), res_o_menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (res_o), !status.resolution);

  label7 = gtk_label_new ("Zoom");
  gtk_widget_ref (label7);
  gtk_object_set_data_full (GTK_OBJECT (window), "label7", label7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label7);
  gtk_fixed_put (GTK_FIXED (fixed), label7, 80, 48);
  gtk_widget_set_uposition (label7, 80, 48);
  gtk_widget_set_usize (label7, 56, 16);
  gtk_label_set_justify (GTK_LABEL (label7), GTK_JUSTIFY_LEFT);
  gtk_label_set_line_wrap (GTK_LABEL (label7), TRUE);

  zoom_o = gtk_option_menu_new ();
  gtk_widget_ref (zoom_o);
  gtk_object_set_data_full (GTK_OBJECT (window), "zoom_o", zoom_o,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (zoom_o);
  gtk_fixed_put (GTK_FIXED (fixed), zoom_o, 80, 64);
  gtk_widget_set_uposition (zoom_o, 80, 64);
  gtk_widget_set_usize (zoom_o, 88, 24);
  zoom_o_menu = gtk_menu_new ();
  glade_menuitem = gtk_menu_item_new_with_label ("58 mm");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (zoom_o_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("51 mm");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (zoom_o_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("41 mm");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (zoom_o_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("34 mm");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (zoom_o_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("29 mm");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (zoom_o_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("Macro");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (zoom_o_menu), glade_menuitem);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (zoom_o), zoom_o_menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (zoom_o), status.zoom_position);
  gtk_widget_show(window);
  
}

int kodak_dc210_configure () 
{
  create_dc210();
  return 1;
}

char *kodak_dc210_summary() 
{
  static char summary_string[2048] = "";
  char buff[1024];
  int serialdev = 0;
  struct kodak_dc210_status status;

      kodak_dc210_get_camera_status (&status);
      strcpy(summary_string,"Camera Status:\n");
      snprintf(buff,1024,"Kodak DC210 [%02d]\n",status.camera_type);
      strcat(summary_string,buff);
      snprintf(buff,1024,"Camera ID: %s\n",status.camera_id);
      strcat(summary_string,buff);
      snprintf(buff,1024,"Firmware: %1d.%02d [DSP %1d.%02d muDSP %1d.%02d]\n",status.firmware_major,status.firmware_minor,status.dsp_major,status.dsp_minor,status.mu_major,status.mu_minor);
      strcat(summary_string,buff);
      snprintf(buff,1024,"Battery [AC] Status: %s [%s]\n",status.battery ? "Low" : "OK", status.acstatus ? "Connected" : "Not Connected");
      strcat(summary_string,buff);
      strftime(buff,1024,"Time: %a, %d %b %Y %T\n",gmtime((time_t *)&status.camera_time));
      strcat(summary_string,buff);
      snprintf(buff,1024,"Zoom Position: %s, timer is %s\n",zoomstr[status.zoom_position],status.timer_mode ? "on" : "off");
      strcat(summary_string,buff);
      snprintf(buff,1024,"Flash mode: %s, flash %s\n",flashstr[status.flash_mode], status.flash_status ? "charged" : "not charged");
      strcat(summary_string,buff);
      snprintf(buff,1024,"Quality: %s, Resolution: %s\n",qualstr[status.compression],status.resolution ? "1152x768" : "640x480");
      strcat(summary_string,buff);
      snprintf(buff,1024,"Total Pictures [Flashes]: %d [%d]\n",status.total_pictures,status.total_flash);
      strcat(summary_string,buff);
      snprintf(buff,1024,"Video Format: %s, Comm mode: %s\n",status.video_format ? "PAL" : "NTSC",status.comm_mode ? "IRDA" : "Serial");
      strcat(summary_string,buff);
      snprintf(buff,1024,"Pictures in Camera: %d\n",status.pictures_in_camera);
      strcat(summary_string,buff);
      snprintf(buff,1024,"Pictures remaining:\n");
      strcat(summary_string,buff);
      snprintf(buff,1024,"High: %d Medium: %d Low: %d\n",status.remaining_low,status.remaining_med,status.remaining_high);
      strcat(summary_string,buff);
  return (summary_string);
}

char *kodak_dc210_description() 
{

  return("Enhanced Kodak DC210 support
by TimeCop <timecop@japan.co.jp>
http://www.ne.jp/asahi/linux/timecop
based on original code
by Brian Hirt <bhirt@mobygames.com> 
http://www.mobygames.com");

}

/* define function pointers for the dc210 camera */
struct _Camera kodak_dc210 = {
		kodak_dc210_initialize,
		kodak_dc210_get_picture,
		kodak_dc210_get_preview,
		kodak_dc210_delete_picture,
		kodak_dc210_take_picture,
		kodak_dc210_number_of_pictures,
		kodak_dc210_configure,
		kodak_dc210_summary,
		kodak_dc210_description};
