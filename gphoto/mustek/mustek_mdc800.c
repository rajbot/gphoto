#include "mustek_mdc800.h"

#include "../src/gphoto.h"
#include "../src/util.h"
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include <glib.h>
#include <gdk_imlib.h>
#include <gdk/gdk.h>

#include <errno.h>

#include <fcntl.h>

#ifdef HAVE_SELECT_H
#include <sys/select.h>
#endif /* HAVE_SELECT_H */
#define TIME_WITH_SYS_TIME
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

static int mustek_dev = -1;       /* global stuff 
				     camera timings make it slow to keep opening and closing the
                                     port (1 sec timeout for each), that's why i need the global
                                     dev */

/* viciosly stolen from the kodak_dc210.c */
int mustek_mdc800_read ( int serialdev, unsigned char *buf, int nbytes )
{
  int ret;
  int numRead = 0;
  int n;
  int len;
  fd_set readfds;
  struct timeval timeout;

  while (numRead < nbytes)
  {
    /* set parameters for select call */
    n = serialdev + 1;
    FD_ZERO(&readfds);
    FD_SET(serialdev, &readfds);

    /* declare a timeout of 1.500sec */
    timeout.tv_sec = 1;
    timeout.tv_usec = 500000;

    ret = select(n, &readfds, NULL, NULL, &timeout);

    /* if the select failed or timed out, hanled it */
    if(ret <= 0)
    {
      if (ret<0)
      {
	/* if the system call was interupted, retry */
	if (errno == EINTR) 
	  continue;
        else
	  perror("select");
      }
       else
	fprintf(stderr, "mustek_mdc800_read: read timed out\n");

      return FALSE;
    }
    else
    {
      if (FD_ISSET(serialdev, &readfds))
      {
	len = read(serialdev, buf + numRead,
		   nbytes - numRead);

        if (len < 0)
        {
	  if (errno == EINTR) 
	    continue;
          else
          { 
	    fprintf(stderr, "mustek_mdc800_read: read failed on a ready file handle\n");
	    perror("read");
	    return FALSE; 
          }
        }
        else
        {
	  numRead += len;
        }
      }
      else
	return FALSE;
    }
  }

  return TRUE;
}

/* viciosly stolen from the kodak_dc210.c */
int mustek_mdc800_write_byte ( int serialdev, char b )
{
/*
This function tries to write the supplied byte to the serial device.

Args
  int serieldev - a file handle
  char b - the byte to write
Returns
  success - 1 if the command completed, 0 otherwise
*/
  int numwrite;

  /* write the bite to the serial device */
  numwrite = write(serialdev, &b, 1);

  /* check to make sure the byte was written */
  if (numwrite != 1)
  {
    fprintf(stderr,"failed to write byte\n");
    return(0);
  }
  else
  {
    return(1);
  }
}
/* stolen from the kodak_dc210.c 
   adapted to the command structure of the mustek */
int mustek_mdc800_send_command ( char serialdev, char command, char arg1, char arg2, char arg3, int count)
{
  unsigned char ack[10];
  int success = TRUE;

  bzero(ack,10);

  success = success && mustek_mdc800_write_byte ( serialdev, 0x55 );
  success = mustek_mdc800_read( serialdev, ack, 1 );
  while (*ack == 0xbb) {  /* any lingering 0xbb ? */
    success = mustek_mdc800_read( serialdev, ack, 1 );  /* read it */
  }
  if (*ack != 0x55) { printf("echo mismatch init: %x:0x55\n", *ack);}
  success = success && mustek_mdc800_write_byte ( serialdev, command );
  success = mustek_mdc800_read( serialdev, ack, 1 );
  if (*ack != command) { printf("echo mismatch cmd: %x:%x\n", *ack,command);}
  success = success && mustek_mdc800_write_byte ( serialdev, arg1 );
  success = mustek_mdc800_read( serialdev, ack, 1 );
  if (*ack != arg1) { printf("echo mismatch arg1: %x:%x\n", *ack,arg1);}
  success = success && mustek_mdc800_write_byte ( serialdev, arg2 );
  success = mustek_mdc800_read( serialdev, ack, 1 );
  if (*ack != arg2) { printf("echo mismatch arg2: %x:%x\n", *ack,arg2);}
  success = success && mustek_mdc800_write_byte ( serialdev, arg3 );
  success = mustek_mdc800_read( serialdev, ack, 1 );
  if (*ack != arg3) { printf("echo mismatch arg3: %x:%x\n", *ack,arg3);}
  success = success && mustek_mdc800_write_byte ( serialdev, 0xAA );
  success = mustek_mdc800_read( serialdev, ack, 1 );
  if (*ack != 0xaa) { printf("echo mismatch end: %x:0xaa\n", *ack);}

  /* if the command was sent successfully to the camera, continue */
  if (success) {
    if (count>0) {
      if (success) {
	/* read ack from camera */
	success = mustek_mdc800_read( serialdev, ack, count );
	
	if (success) {
     
	  /* make sure the ack is okay */
	  if (ack[count-1] != MDC_COMMAND_ACK)
	    {
	      fprintf(stderr,"mustek_mdc800_send_command - bad ack from camera\n");
	      success = FALSE;
	    }
	}
      }
      else
	{
	  fprintf(stderr,"mustek_mdc800_send_command - failed to read ack from camera\n");
	  success = FALSE;
	}
    }
  } 
  else
    {
      fprintf(stderr,"mustek_mdc800_send_command - failed to send command to camera\n");
      success = FALSE;
    }
  return(success);
}

/* stolen from the kodak_dc210.c 
   adapted to the mustek still some bugs */
int mustek_mdc800_set_port_speed(int serialdev,int speed)
{
  int success = TRUE;
  char arg1, arg2, ack[2];
  struct termios newt, oldt;
  speed_t old_speed;

  /* get the termios structure */
  if (tcgetattr(serialdev, &oldt) < 0) 
  {
    success = 0;
    error_dialog("tcgetattr");
  }
  else
  {
  
    /* make a copy of the old termios structure */
    memcpy((char *)&newt,(char *)&oldt, sizeof(struct termios));

    old_speed = cfgetospeed(&oldt);
#ifdef MUSTEK_DEBUG
    printf("Old speed: %d\n",old_speed);
    printf("Set speed: %d\n",speed);
#endif

    switch (old_speed) {
    case B19200:
      arg1=0x0;
      break;
    case B57600:
      arg1=0x1;
      break;
    case B115200:
      arg1=0x2;
      break;
    default:
      error_dialog("Invalid old speed");
    }
    if (speed == 19200)
    {
      arg2 = 0x0;
      cfsetospeed(&newt, B19200);
      cfsetispeed(&newt, B19200);
    }
    else if (speed == 57600)
    {
      arg2 = 0x1;
      cfsetospeed(&newt, B57600);
      cfsetispeed(&newt, B57600);
    }
    else if (speed == 115200)
    {
      arg2 = 0x2;
      cfsetospeed(&newt, B115200);
      cfsetispeed(&newt, B115200);
    }
    else
    {
      success = 0;
      fprintf(stderr,"speed not supported %d",speed);
    }

    if (success) {  /* old speed */
      success = mustek_mdc800_send_command(serialdev,MDC_SET_SPEED,arg2,arg1,0x00,0);
      sleep(1);
      if (success)
      { 
	if (tcsetattr(serialdev, TCSANOW, &newt) < 0) 
        {
	  error_dialog("Serial speed change problem");
	  success = 0;
        }
      }
      mustek_mdc800_read( serialdev, ack, 1 ); /* new speedy speed */
      success = mustek_mdc800_send_command(serialdev,MDC_SET_SPEED,arg2,arg2,0x00,0);
    }
  }
  sleep(1);
  return(success);
}

/* stolen from the kodak_dc210.c 
   device is a global var */
int mustek_mdc800_open_camera (const char *devname)
{
  int serialdev;

  struct termios newt, oldt;
  struct stat dummy;
#ifdef MUSTEK_DEBUG
  printf("mustek_mdc800_open_camera\n");
#endif
  if (mustek_dev < 0) {
#ifdef MUSTEK_DEBUG
    printf("mustek_mdc800_open_camera: OPEN\n");
#endif
    serialdev = open(devname, O_RDWR|O_NOCTTY);

    if (serialdev < 0) 
      error_dialog("Cannot open device");
    
    if (tcgetattr(serialdev, &oldt) < 0) 
      error_dialog("tcgetattr");
    
    memcpy((char *)&newt,(char *)&oldt, sizeof(struct termios));
    
    cfmakeraw(&newt);
    newt.c_oflag &= ~CSTOPB;
    newt.c_cflag &= ~PARENB;
    newt.c_cflag |= CS8;
    newt.c_cflag &= ~PARODD;
    
    newt.c_cc[VMIN] = 0;
    newt.c_cc[VTIME] = 10;
    
    cfsetospeed(&newt, B57600);
    cfsetispeed(&newt, B57600);
    if (tcsetattr(serialdev, TCSANOW, &newt) < 0) 
      error_dialog("Serial speed change problem");
    /* lock the buttons on the camera */
    mustek_mdc800_send_command(serialdev,MDC_INITIALIZE,0x00,0x00,0x00,9);  
    sleep(1);
    /* FIXME speed hardcoded */
    mustek_mdc800_set_port_speed(serialdev,115200);
    sleep(1); 
    mustek_dev = serialdev;
    return mustek_dev;
  } else {
#ifdef MUSTEK_DEBUG
    printf("mustek_mdc800_open_camera: CACHED\n");
#endif
    return mustek_dev;
  }
}

/* not doing anything at the moment */
int mustek_mdc800_close_camera (int serialdev) 
{
  struct timespec timeout;

  /*  printf("mustek_mdc800_close_camera\n"); 
  timeout.tv_sec = 0;
  timeout.tv_nsec = 4000000;

  nanosleep(&timeout, NULL);
  mustek_mdc800_send_command(serialdev,MDC_DISCONNECT,0x00,0x00,0x00,1); 
  /* mustek_mdc800_set_port_speed(serialdev,57600); */
  /*  close(serialdev);  */
  return(1);
}

/* lights please */
int mustek_mdc800_lcd_on (int serialdev) {
  struct timespec timeout;

#ifdef MUSTEK_DEBUG
  printf("mustek_mdc800_lcd_on\n");
#endif

  timeout.tv_sec = 0;
  timeout.tv_nsec = 3000000;
  nanosleep(&timeout, NULL);
  mustek_mdc800_send_command(serialdev,MDC_LCDON,0x00,0x00,0x00,1);
  return(0);
}

/* save some battery */
int mustek_mdc800_lcd_off (int serialdev) {
  struct timespec timeout;

#ifdef MUSTEK_DEBUG
  printf("mustek_mdc800_lcd_off\n");
#endif

  timeout.tv_sec = 0;
  timeout.tv_nsec = 3000000;
  nanosleep(&timeout, NULL);
  mustek_mdc800_send_command(serialdev,MDC_LCDOFF,0x00,0x00,0x00,1);
  return(0);
}

/* checksum to send as ack after each 512bytes during transfer */
int sum (char picdata[512]) {
  int i=0;
  char soma=0;
  while (i < 512) {
    soma = soma + picdata[i];
    i++;
  }
  return(soma);
}

int mustek_mdc800_initialize() 
{
#ifdef MUSTEK_DEBUG
  printf("mustek_mdc800_initialize\n");
#endif
  if (mustek_mdc800_open_camera(serial_port)) {  /* open port */
    sleep(1);
    mustek_mdc800_send_command(mustek_dev,MDC_SET_AUTOFF_NO,0,0,0,1);  /* don't shutdown */
    return(1);
  } else {
    return(0);
  }
}

/* what's this for ? */
char *mustek_mdc800_summary() 
{
#ifdef MUSTEK_DEBUG
  printf("mustek_mdc800_summary\n");
#endif
  return("Not Supported");
}

char *mustek_mdc800_description() 
{
#ifdef MUSTEK_DEBUG
  printf("mustek_mdc800_description\n");
#endif
  return("Mustek MDC-800 Digital Camera Support\n\nPedro Miguel Caria <pmiguel@maquina.com>\nJose Gabriel Marcelino <gabriel@maquina.com>\n\nSerial port support\nEconomic: 506x384\nStandard and High: 1012x768\n");
}

/* some parts stolen from kodak_dc210.c */
struct Image *mustek_mdc800_get_picture ( int picNum, int thumbnail) {

  struct Image *im = NULL;

  char buffer[512];
  char checksum[1];
  char realsize[4];
  int fileSize;
  char *picData;
  char dummy;
  char header[] = FILE_HEADER;
  char header1[] = FILE_HEADER1;
  char huffman[] = HUFFMAN_TAB;
  char sos1[] = SOF_SOS1;
  char sos2[] = SOF_SOS2;
  char sos3[] = SOF_SOS3;
  int numRead;
  int quality=ECONOMIC;
  unsigned int max=8;     /* 4kb default, god for thumbnails */
  int n1=0,n2=0,n3=0;
  struct timespec timeout;

#ifdef MUSTEK_DEBUG
  printf("mustek_mdc800_get_picture\n"); 
#endif
  if (mustek_mdc800_open_camera(serial_port)) { /* if port closed, open */
    n1 = div(picNum, 10).rem;  /* transform picNum in BCD */
    n2 = div(div(picNum, 10).quot,10).rem;
    n3 = div(div(picNum, 10).quot,10).quot;
    update_progress(0.00);
    if (thumbnail) {
      timeout.tv_sec = 0;
      timeout.tv_nsec = 200000;
      nanosleep(&timeout, NULL); /* don't have the timings, done by trial and error */
      mustek_mdc800_send_command(mustek_dev,MDC_SETTARGET,TARGET_THUMBNAIL,0x0,0x0,1);
      timeout.tv_sec = 1;
      timeout.tv_nsec = 0;
      nanosleep(&timeout, NULL);
      mustek_mdc800_send_command(mustek_dev,MDC_GETTHUMBNAIL,n3,n2,n1,0);
      max = 8;  /* thumbnails are always 4kb */
      /* ++++ */
      fileSize = 512 * max;
      picData = (char *)malloc(fileSize+512);
      numRead = 0;
      while (numRead < fileSize) {
	mustek_mdc800_read(mustek_dev,buffer,512);
	dummy = sum(buffer);
	mustek_mdc800_write_byte(mustek_dev,dummy);
       	mustek_mdc800_read(mustek_dev,checksum,1);
	if (checksum[0] == dummy) {    /* correcto ? */ 
	  memcpy(picData+numRead,buffer,512);
	  numRead += 512;
	  if (numRead <= fileSize) {
	    update_progress((float)numRead / (float)fileSize);
	  }
	}
      }
      memcpy(picData,header,41);
      *(picData+105) = 1;
      memcpy(picData+170,huffman,424);
      memcpy(picData+991,sos1,33);
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
      /* ++++ */
    } else {
      mustek_mdc800_lcd_off(mustek_dev);  /* lcd off to avoid flicker */
      timeout.tv_sec = 0;
      timeout.tv_nsec = 600000;
      nanosleep(&timeout, NULL);
      mustek_mdc800_send_command(mustek_dev,MDC_SETTARGET,TARGET_IMAGE,0x0,0x0,1);
      sleep(1);
      mustek_mdc800_send_command(mustek_dev,MDC_GETIMAGE,n3,n2,n1,0);
      timeout.tv_sec = 0;
      timeout.tv_nsec = 600000;
      nanosleep(&timeout, NULL);
      mustek_mdc800_read(mustek_dev,buffer,512);
      dummy = sum(buffer);
      mustek_mdc800_write_byte(mustek_dev,dummy);
      mustek_mdc800_read(mustek_dev,checksum,1);
      if (checksum[0] == dummy) {    /* correcto ? */ 
	quality = buffer[35];
	/* based on the image quality decide wich jpeg size, the camera
	   always outputs these sizes, even when the picture is smaller,
	   it just fills the rest with 0xff */
	switch (quality) {
	case ECONOMIC:
	  { max=96; break;}    /*  48 Kb */
	case STANDARD:
	  { max=256; break;}   /* 128 Kb */
	case HIGH:
	  { max = 640; break;} /* 320 Kb */
	}
	fileSize = 512 * max;
	picData = (char *)malloc(fileSize+512);
	numRead = 0;
	memcpy(picData+numRead,buffer,512);
	numRead += 512;
      }
      while (numRead < fileSize) {
	mustek_mdc800_read(mustek_dev,buffer,512);
	dummy = sum(buffer);
	mustek_mdc800_write_byte(mustek_dev,dummy);
	mustek_mdc800_read(mustek_dev,checksum,1);
	if (checksum[0] == dummy) {    /* correcto ? */ 
	  memcpy(picData+numRead,buffer,512);
	  numRead += 512;
	  if (numRead <= fileSize) {
	    update_progress((float)numRead / (float)fileSize);
	  }
	}  
      }
      /* jpeg's that come from the camera have the headers wrong
	 so correct them */
      switch (quality) {
      case ECONOMIC:
	{ memcpy(picData+5087,sos2,33); break;}
      case STANDARD:
	{ memcpy(picData+5087,sos3,33); break;}
      case HIGH:
	{ memcpy(picData+5087,sos3,33); break;}
      }
      update_progress(1.00);
      memcpy(picData,header1,24);
      memcpy(picData+4096,header,41);
      *(picData+4096) = 0;
      *(picData+4097) = 0;
      *(picData+4201) = 1;
      memcpy(picData+4266,huffman,424);

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
  }
  return(im);
}

int mustek_mdc800_number_of_pictures() {
  int numPics = 0;
  char pics[2];
  unsigned char check;

#ifdef MUSTEK_DEBUG
  printf("mustek_mdc800_number_of_pictures\n");
#endif
  if (mustek_mdc800_open_camera(serial_port)) {
    mustek_mdc800_send_command(mustek_dev,MDC_GETNOPICS,0x00,0x00,0x00,0);
    sleep(1);
    mustek_mdc800_read(mustek_dev,pics,2);
    sleep(1);
    mustek_mdc800_read(mustek_dev,&check,1);  
    if (check != MDC_COMMAND_ACK) { /* to avoid returning 100000 pics */
      error_dialog("Error reading nr of pics\n");
      return(0);
    }
    numPics = pics[0]*256+pics[1];
  }
  return(numPics);
}

/* its not here yet */
struct Image *mustek_mdc800_get_preview () 
{
#ifdef MUSTEK_DEBUG
  printf("mustek_mdc800_get_preview\n");
#endif
  return(0);
}

int mustek_mdc800_take_picture () 
{
  int initPic = 0;
  int numPics = 0;
  char pics[2];
  unsigned char check;

#ifdef MUSTEK_DEBUG
  printf("mustek_mdc800_take_picture\n");
#endif
  initPic = mustek_mdc800_number_of_pictures();
  if (mustek_mdc800_open_camera(serial_port)) {
    sleep(1);
    mustek_mdc800_send_command(mustek_dev,MDC_TAKEPICTURE,0,0,0,0);
    sleep(2);
    mustek_mdc800_send_command(mustek_dev,MDC_GETNOPICS,0x00,0x00,0x00,0);
    sleep(1);
    mustek_mdc800_read(mustek_dev,pics,1);
    sleep(1);
    numPics = mustek_mdc800_number_of_pictures();
    if (numPics > initPic) {  /* picture taken ? */
      return(numPics);
    } else {
      error_dialog("No picture taken\nMemory full ?\n");
      return(0);
    }
  } else {
    return(0);
  }
}

int mustek_mdc800_delete_image (int picture_number) {
  int numPics = 0;
  int n1=0,n2=0,n3=0;

#ifdef MUSTEK_DEBUG
  printf("mustek_mdc800_delete_image\n");
#endif
  n1 = div(picture_number, 10).rem;
  n2 = div(div(picture_number, 10).quot,10).rem;
  n3 = div(div(picture_number, 10).quot,10).quot;
  if (mustek_mdc800_open_camera(serial_port)) {
    sleep(1);
    mustek_mdc800_send_command(mustek_dev,MDC_DELETEPICTURE,n3,n2,n1,1);
    sleep(1);
    numPics = mustek_mdc800_number_of_pictures();
    return(numPics);
  } else {
    return(0);
  }
}

/* greatly stolen from olympus.c */
int mustek_mdc800_configure () {
	/*
	   Shows the Mustek config dialog
	*/

	char *info, *camID;
	off_t info_size = 2048;
	time_t camtime;
	char *atime;
	char exposure[3];
	char status[4];
	char quality[2];
	long value;

	GtkWidget *dialog, *table, *label, *spacer, *toggle;
	GtkWidget *save_button, *cancel_button;
	GtkObject *adj;
	GtkAdjustment *adjustment;
	GSList *group;

	struct ConfigValues {
		GtkWidget *cam_id;
	        GtkWidget *qual_econ;
	        GtkWidget *qual_std;
		GtkWidget *qual_high;
	        GtkWidget *lcd;
	        GtkWidget *autoff_on;
		GtkWidget *autoff_off;
	        GtkWidget *light_auto;
	        GtkWidget *light_indoor;
	        GtkWidget *light_indoorplus;
	        GtkWidget *light_outdoor;
	        GtkWidget *flash_auto;
		GtkWidget *flash_on;
		GtkWidget *flash_off;
	        GtkWidget *lcd_on;
	 	GtkWidget *lcd_off;
	        GtkWidget *menu_on;
		GtkWidget *menu_off;
		GtkWidget *source_card;
		GtkWidget *source_mem;
		GtkWidget *exp_mtrx;
		GtkWidget *exp_cntr;
	} Config;

#ifdef MUSTEK_DEBUG
	printf("mustek_mdc800_configure\n");
#endif

	info = malloc(2048);

	update_status("Getting Camera Configuration...");

	/* dialog window ---------------------- */
	dialog = gtk_dialog_new();
	gtk_window_set_title (GTK_WINDOW(dialog), "Configure Camera");
	gtk_container_border_width(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),
				   10);
	table = gtk_table_new(15,5,FALSE);
	gtk_widget_show(table);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), table);

	spacer = gtk_vseparator_new();
	gtk_widget_show(spacer);
	gtk_table_attach_defaults(GTK_TABLE(table),spacer,2,3,2,21);	
      
	if (mustek_dev = mustek_mdc800_open_camera(serial_port)) {
	} else {
	  error_dialog("Could not open camera.");
	  return 0;
	}
	sleep(1);
	mustek_mdc800_send_command(mustek_dev,MDC_GETSTATUS,0,0,0,0);
	mustek_mdc800_read(mustek_dev,status,5);

	/* camera id ---------------------- */
	label = gtk_label_new("Camera ID:");
	gtk_widget_show(label);
	Config.cam_id = gtk_entry_new();
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,0,1);	
	gtk_table_attach_defaults(GTK_TABLE(table),Config.cam_id,1,5,0,1);

	spacer = gtk_hseparator_new();
	gtk_widget_show(spacer);
	gtk_table_attach_defaults(GTK_TABLE(table),spacer,0,5,1,2);

	info[0] = '\0'; info_size=2048;

	update_progress(.1);
	gtk_entry_set_text(GTK_ENTRY(Config.cam_id), info);

	/* image quality ---------------------- */
	label = gtk_label_new("Image Quality:");
	gtk_widget_show(label);

	Config.qual_econ = gtk_radio_button_new_with_label(NULL, "Economic");
	gtk_widget_show(Config.qual_econ);
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(Config.qual_econ));

	Config.qual_std = gtk_radio_button_new_with_label(group, "Standard");
	gtk_widget_show(Config.qual_std);
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(Config.qual_std));

	Config.qual_high = gtk_radio_button_new_with_label(group, "High");
	gtk_widget_show(Config.qual_high);

	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,2,3);
	gtk_table_attach_defaults(GTK_TABLE(table),Config.qual_econ,1,2,2,3);	
	gtk_table_attach_defaults(GTK_TABLE(table),Config.qual_std,1,2,3,4);	
	gtk_table_attach_defaults(GTK_TABLE(table),Config.qual_high,1,2,4,5);

	spacer = gtk_hseparator_new();
	gtk_widget_show(spacer);
	gtk_table_attach_defaults(GTK_TABLE(table),spacer,0,2,5,6);
	sleep(1);
	mustek_mdc800_send_command(mustek_dev,MDC_GETQUALITY,0,0,0,0);
	mustek_mdc800_read(mustek_dev,quality,2);
	update_progress(.2);
	value = quality[0];
	switch (value) {
		case 0:
			gtk_widget_activate(Config.qual_econ);
			break;
		case 1:
			gtk_widget_activate(Config.qual_std);
			break;
		default:
			gtk_widget_activate(Config.qual_high);
	}

	/* exposure seting ---------------------- */
	label = gtk_label_new("Exposure:");
	gtk_widget_show(label);
	value = 2;
	sleep(1);
	mustek_mdc800_send_command(mustek_dev,MDC_GETWB,0,0,0,0);
	mustek_mdc800_read(mustek_dev,exposure,3);
	value = exposure[0]-2;
	adj = gtk_adjustment_new(value, -2, 2, 1, 0, 0);
	Config.lcd = gtk_hscale_new(GTK_ADJUSTMENT(adj));
	gtk_range_set_update_policy(GTK_RANGE(Config.lcd),GTK_UPDATE_CONTINUOUS);
	gtk_scale_set_draw_value(GTK_SCALE(Config.lcd), TRUE);
	gtk_scale_set_digits(GTK_SCALE(Config.lcd), 0);
	gtk_widget_show(Config.lcd);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,6,7);	
	gtk_table_attach_defaults(GTK_TABLE(table),Config.lcd,0,2,7,8);	

	spacer = gtk_hseparator_new();
	gtk_widget_show(spacer);
	gtk_table_attach_defaults(GTK_TABLE(table),spacer,0,2,8,9);
	update_progress(.3);

	/* light source ---------------------- */
	label = gtk_label_new("Light source:");
	gtk_widget_show(label);
	Config.light_auto = gtk_radio_button_new_with_label(NULL, "Auto");
	gtk_widget_show(Config.light_auto);
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(Config.light_auto));
	Config.light_indoor = gtk_radio_button_new_with_label(group, "Indoor");
	gtk_widget_show(Config.light_indoor);
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(Config.light_indoor));
	Config.light_indoorplus = gtk_radio_button_new_with_label(group, "Indoor+");
	gtk_widget_show(Config.light_indoorplus);
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(Config.light_indoorplus));
	Config.light_outdoor = gtk_radio_button_new_with_label(group, "Outdoor");
	gtk_widget_show(Config.light_outdoor);

	gtk_table_attach_defaults(GTK_TABLE(table),label,3,4,7,8);
	gtk_table_attach_defaults(GTK_TABLE(table),Config.light_auto,4,5,7,8);	
	gtk_table_attach_defaults(GTK_TABLE(table),Config.light_indoor,4,5,8,9);	
	gtk_table_attach_defaults(GTK_TABLE(table),Config.light_indoorplus,4,5,9,10);
	gtk_table_attach_defaults(GTK_TABLE(table),Config.light_outdoor,4,5,10,11);

	spacer = gtk_hseparator_new();
	gtk_widget_show(spacer);
	gtk_table_attach_defaults(GTK_TABLE(table),spacer,3,5,12,13);

	update_progress(.4);
	value = exposure[1];
	switch (value) {
		case 2:
			gtk_widget_activate(Config.light_indoor);
			break;
		case 4:
			gtk_widget_activate(Config.light_indoorplus);
			break;
		case 8:
			gtk_widget_activate(Config.light_outdoor);
			break;
		default:
			gtk_widget_activate(Config.light_auto);
	}
	/* flash mode ---------------------- */
	label = gtk_label_new("Flash Mode:");
	gtk_widget_show(label);
	Config.flash_auto = gtk_radio_button_new_with_label(NULL, "Auto");
	gtk_widget_show(Config.flash_auto);
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(Config.flash_auto));
	Config.flash_on = gtk_radio_button_new_with_label(group, "ON");
	gtk_widget_show(Config.flash_on);
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(Config.flash_on));
	Config.flash_off = gtk_radio_button_new_with_label(group, "OFF");
	gtk_widget_show(Config.flash_off);
	gtk_table_attach_defaults(GTK_TABLE(table),label,3,4,2,3);	
	gtk_table_attach_defaults(GTK_TABLE(table),Config.flash_auto,4,5,2,3);
	gtk_table_attach_defaults(GTK_TABLE(table),Config.flash_on,4,5,3,4);	
	gtk_table_attach_defaults(GTK_TABLE(table),Config.flash_off,4,5,4,5);	
	
	spacer = gtk_hseparator_new();
	gtk_widget_show(spacer);
	gtk_table_attach_defaults(GTK_TABLE(table),spacer,3,5,6,7);	
	value = 0;
	update_progress(.5);
	if ((status[3] & 2) == 2)
	  value=1;
	if ((status[3] & 4) == 4)
	  value=2;
	if ((status[3] & 0) == 6)
	  value=0;
	switch (value) {
	        case 1:
			gtk_widget_activate(Config.flash_on);
			break;
		case 2:
			gtk_widget_activate(Config.flash_off);
			break;
		default:
			gtk_widget_activate(Config.flash_auto);
	}

	/* LCD ON/OFF ---------------------- */
	label = gtk_label_new("LCD:");
	gtk_widget_show(label);
	Config.lcd_on = gtk_radio_button_new_with_label(NULL, "ON");
	gtk_widget_show(Config.lcd_on);
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(Config.lcd_on));
	Config.lcd_off = gtk_radio_button_new_with_label(group, "OFF");
	gtk_widget_show(Config.lcd_off);
	gtk_table_attach_defaults(GTK_TABLE(table),label,3,4,19,20);
	gtk_table_attach_defaults(GTK_TABLE(table),Config.lcd_on,4,5,19,20);
	gtk_table_attach_defaults(GTK_TABLE(table),Config.lcd_off,4,5,20,21);	
	
	spacer = gtk_hseparator_new();
	gtk_widget_show(spacer);
	gtk_table_attach_defaults(GTK_TABLE(table),spacer,3,5,22,23);
	value = 0;
	update_progress(.6);
	if ((status[1] & 4) == 4)
	  value=1;
	else
	  value=0;
	switch (value) {
	case 1:
	  gtk_widget_activate(Config.lcd_on);
	  break;
	default:
	  gtk_widget_activate(Config.lcd_off);
	}

	/* Menu ON/OFF ---------------------- */
	label = gtk_label_new("Menu Display:");
	gtk_widget_show(label);
	Config.menu_on = gtk_radio_button_new_with_label(NULL, "ON");
	gtk_widget_show(Config.menu_on);
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(Config.menu_on));
	Config.menu_off = gtk_radio_button_new_with_label(group, "OFF");
	gtk_widget_show(Config.menu_off);
	gtk_table_attach_defaults(GTK_TABLE(table),label,3,4,14,15);
	gtk_table_attach_defaults(GTK_TABLE(table),Config.menu_on,4,5,14,15);
	gtk_table_attach_defaults(GTK_TABLE(table),Config.menu_off,4,5,15,16);	
	
	spacer = gtk_hseparator_new();
	gtk_widget_show(spacer);
	gtk_table_attach_defaults(GTK_TABLE(table),spacer,3,5,17,18);
	value = 0;
	update_progress(.7);
	if ((status[1] & 1) == 1)
	  value=1;
	else
	  value=0;
	switch (value) {
	case 1:
	  gtk_widget_activate(Config.menu_on);
	  break;
	default:
	  gtk_widget_activate(Config.menu_off);
	}

	/* Auto-Off ON/OFF ---------------------- */
	label = gtk_label_new("Auto-Off:");
	gtk_widget_show(label);
	Config.autoff_on = gtk_radio_button_new_with_label(NULL, "ON");
	gtk_widget_show(Config.autoff_on);
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(Config.autoff_on));
	Config.autoff_off = gtk_radio_button_new_with_label(group, "OFF");
	gtk_widget_show(Config.autoff_off);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,14,15);
	gtk_table_attach_defaults(GTK_TABLE(table),Config.autoff_on,1,2,14,15);
	gtk_table_attach_defaults(GTK_TABLE(table),Config.autoff_off,1,2,15,16);	
	
	spacer = gtk_hseparator_new();
	gtk_widget_show(spacer);
	gtk_table_attach_defaults(GTK_TABLE(table),spacer,0,2,17,18);
	value = 0;
	update_progress(.8);
	if ((status[1] & 8) == 8)
	  value=1;
	else
	  value=0;
	switch (value) {
	case 1:
	  gtk_widget_activate(Config.autoff_on);
	  break;
	default:
	  gtk_widget_activate(Config.autoff_off);
	}

	/* Storage Source ---------------------- */
	label = gtk_label_new("Storage-Source:");
	gtk_widget_show(label);
	Config.source_card = gtk_radio_button_new_with_label(NULL, "CF-Card");
	gtk_widget_show(Config.source_card);
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(Config.source_card));
	Config.source_mem = gtk_radio_button_new_with_label(group, "Internal Memory");
	gtk_widget_show(Config.source_mem);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,19,20);
	gtk_table_attach_defaults(GTK_TABLE(table),Config.source_card,1,2,19,20);
	gtk_table_attach_defaults(GTK_TABLE(table),Config.source_mem,1,2,20,21);	
	
	spacer = gtk_hseparator_new();
	gtk_widget_show(spacer);
	gtk_table_attach_defaults(GTK_TABLE(table),spacer,0,2,22,23);
	value = 0;
	update_progress(.9);
	if ((status[0] & 1) == 1)
	  value=0;
	else
	  value=1;
	switch (value) {
	case 1:
	  gtk_widget_activate(Config.source_card);
	  break;
	default:
	  gtk_widget_activate(Config.source_mem);
	}

	/* Set exposure Mode ---------------------- */
	value = 0;
	sleep(1);
	mustek_mdc800_send_command(mustek_dev,MDC_GET_EXPOSURE_MODE,0,0,0,0);
	mustek_mdc800_read(mustek_dev,exposure,2);
	label = gtk_label_new("Exposure Mode:");
	gtk_widget_show(label);
	Config.exp_mtrx = gtk_radio_button_new_with_label(NULL, "MTRX");
	gtk_widget_show(Config.exp_mtrx);
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(Config.exp_mtrx));
	Config.exp_cntr = gtk_radio_button_new_with_label(group, "Center");
	gtk_widget_show(Config.exp_cntr);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,9,10);
	gtk_table_attach_defaults(GTK_TABLE(table),Config.exp_mtrx,1,2,9,10);
	gtk_table_attach_defaults(GTK_TABLE(table),Config.exp_cntr,1,2,10,11);	
	
	spacer = gtk_hseparator_new();
	gtk_widget_show(spacer);
	gtk_table_attach_defaults(GTK_TABLE(table),spacer,0,2,12,13);
	value = 0;
	update_progress(1);
	if ((exposure[0] & 1) == 1)
	  value=1;
	else
	  value=0;
	switch (value) {
	case 1:
	  gtk_widget_activate(Config.exp_cntr);
	  break;
	default:
	  gtk_widget_activate(Config.exp_mtrx);
	}
	/* done with the widgets */
        toggle = gtk_toggle_button_new();
	gtk_widget_show(toggle);
	gtk_widget_hide(toggle);
        gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
                           toggle, TRUE, TRUE, 0);

	save_button = gtk_button_new_with_label("Save");
	gtk_widget_show(save_button);
	GTK_WIDGET_SET_FLAGS (save_button, GTK_CAN_DEFAULT);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
			   save_button, FALSE, FALSE, 0);
	cancel_button = gtk_button_new_with_label("Cancel");
	gtk_widget_show(cancel_button);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
			   cancel_button, FALSE, FALSE, 0);
	gtk_widget_grab_default (save_button);

	gtk_object_set_data(GTK_OBJECT(dialog), "button", "CANCEL");
	gtk_widget_show(dialog);
	update_status("Done.");
	update_progress(0);

	if (wait_for_hide(dialog, save_button, cancel_button) == 0)
	  return (1);

	update_status("Saving Configuration...");

	if (mustek_mdc800_open_camera(serial_port)) {
	} else {
	  error_dialog("Could not open camera.");
	  return 0;
	}

	/* Set image quality... */
	value = 0;
	if (GTK_WIDGET_STATE(Config.qual_econ) == GTK_STATE_ACTIVE)
	  value = 0;
	else if (GTK_WIDGET_STATE(Config.qual_std) == GTK_STATE_ACTIVE)
	  value = 1;
	else if (GTK_WIDGET_STATE(Config.qual_high) == GTK_STATE_ACTIVE)
	  value = 2;
	sleep(1);
	mustek_mdc800_send_command(mustek_dev,MDC_SETQUALITY,value,0,0,1);
	update_progress(0.125);

	/* set exposure setting */

	adjustment = gtk_range_get_adjustment(GTK_RANGE(Config.lcd));
	value = adjustment->value + 2;
	sleep(1);
	mustek_mdc800_send_command(mustek_dev,MDC_SETEXPOSURE,value,0,0,1);
	update_progress(0.25);

	/* Set flash mode... */
	sleep(1);
	value = 0;
	if (GTK_WIDGET_STATE(Config.flash_auto) == GTK_STATE_ACTIVE)
	  mustek_mdc800_send_command(mustek_dev,MDC_SET_FLASHAUTO,value,0,0,1);
	else if (GTK_WIDGET_STATE(Config.flash_on) == GTK_STATE_ACTIVE)
	  mustek_mdc800_send_command(mustek_dev,MDC_SET_FLASHON,value,0,0,1);
	else if (GTK_WIDGET_STATE(Config.flash_off) == GTK_STATE_ACTIVE)
	  mustek_mdc800_send_command(mustek_dev,MDC_SET_FLASHOFF,0,0,0,1);
	update_progress(0.375);

	/* Set light source... */
	value = 0;
	if (GTK_WIDGET_STATE(Config.light_auto) == GTK_STATE_ACTIVE)
	  value = 1;
	else if (GTK_WIDGET_STATE(Config.light_indoor) == GTK_STATE_ACTIVE)
	  value = 2;
	else if (GTK_WIDGET_STATE(Config.light_indoorplus) == GTK_STATE_ACTIVE)
	  value = 4;
	else if (GTK_WIDGET_STATE(Config.light_outdoor) == GTK_STATE_ACTIVE)
	  value = 8;
	sleep(1);
	mustek_mdc800_send_command(mustek_dev,MDC_SET_LIGHT,value,0,0,1);
	update_progress(0.5);

	/* Set LCD ON/OFF... */
	sleep(1);
	if (GTK_WIDGET_STATE(Config.lcd_on) == GTK_STATE_ACTIVE)
	  mustek_mdc800_lcd_on(mustek_dev);
	else if (GTK_WIDGET_STATE(Config.lcd_off) == GTK_STATE_ACTIVE)
	  mustek_mdc800_lcd_off(mustek_dev);
	update_progress(0.625);

	/* Set Menu ON/OFF... */
	sleep(1);
	if (GTK_WIDGET_STATE(Config.menu_on) == GTK_STATE_ACTIVE)
	  mustek_mdc800_send_command(mustek_dev,MDC_SET_MENUON,0,0,0,1);
	else if (GTK_WIDGET_STATE(Config.menu_off) == GTK_STATE_ACTIVE)
	  mustek_mdc800_send_command(mustek_dev,MDC_SET_MENUOFF,0,0,0,1);
	update_progress(0.75);

	/* Set Auto-OFF ON/OFF... */
		sleep(1);
	if (GTK_WIDGET_STATE(Config.autoff_on) == GTK_STATE_ACTIVE)
	  mustek_mdc800_send_command(mustek_dev,MDC_SET_AUTOFF_YES,0,0,0,1);
	else if (GTK_WIDGET_STATE(Config.autoff_off) == GTK_STATE_ACTIVE)
	  mustek_mdc800_send_command(mustek_dev,MDC_SET_AUTOFF_NO,0,0,0,1);
	update_progress(0.875);

	/* Set Storage Source... */
	sleep(1);  
	value = 0;
	if (GTK_WIDGET_STATE(Config.source_card) == GTK_STATE_ACTIVE)
	  value = 0;
	else if (GTK_WIDGET_STATE(Config.source_mem) == GTK_STATE_ACTIVE)
	  value = 1;
	mustek_mdc800_send_command(mustek_dev,MDC_SET_STORAGE,value,0,0,1);
	sleep(1);
	/*	mustek_mdc800_number_of_pictures();  */
	mustek_mdc800_send_command(mustek_dev,MDC_SETTARGET,TARGET_THUMBNAIL,0x0,0x0,1);
	update_progress(0.925);

	/* Set Exposure Mode... */
	sleep(1);  
	value = 0;
	if (GTK_WIDGET_STATE(Config.exp_mtrx) == GTK_STATE_ACTIVE)
	  value = 0;
	else if (GTK_WIDGET_STATE(Config.exp_cntr) == GTK_STATE_ACTIVE)
	  value = 1;
	  mustek_mdc800_send_command(mustek_dev,MDC_SET_EXPOSURE_MODE,value,0,0,1);
	update_progress(1);

	gtk_widget_destroy(dialog);
	update_status("Done.");
	update_progress(0);
	sleep(1);
	return 1;
}

/* define function pointers for the mustek mdc800 camera */
struct _Camera mustek_mdc800 = {
                mustek_mdc800_initialize,
                mustek_mdc800_get_picture,
                mustek_mdc800_get_preview,
                mustek_mdc800_delete_image,
                mustek_mdc800_take_picture,
                mustek_mdc800_number_of_pictures,
                mustek_mdc800_configure,
                mustek_mdc800_summary,
                mustek_mdc800_description};
