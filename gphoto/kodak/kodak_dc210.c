#include "dc210.h"
#include "kodak_dc210.h"

#include "../src/gphoto.h"

#include <stdio.h>
#include <stdlib.h>
#include <byteswap.h>
#include <errno.h>
#include <gdk_imlib.h>
#include <gdk/gdk.h>

int kodak_dc210_send_command ( char serialdev, char command, char arg1, char arg2, char arg3, char arg4)
{
  unsigned char ack;

  kodak_dc210_write_byte ( serialdev, command );
  kodak_dc210_write_byte ( serialdev, 0x00 );
  kodak_dc210_write_byte ( serialdev, arg1 );
  kodak_dc210_write_byte ( serialdev, arg2 );
  kodak_dc210_write_byte ( serialdev, arg3 );
  kodak_dc210_write_byte ( serialdev, arg4 );
  kodak_dc210_write_byte ( serialdev, 0x00 );
  kodak_dc210_write_byte ( serialdev, 0x1A );

  /* Get ack from camera */
  ack = kodak_dc210_read_byte( serialdev );
  if (ack != DC_COMMAND_ACK)
  {
    fprintf(stderr,"kodak_dc210_send_command - bad ack from camera\n");
    return(1);
  }

  return(0);
}

int kodak_dc210_set_port_speed(int serialdev,int speed)
{
  int arg1, arg2;
  struct termios newt, oldt;

  /* get the termios structure */
  if (tcgetattr(serialdev, &oldt) < 0) 
    error_dialog("tcgetattr");

  /* make a copy of the old termios structure */
  memcpy((char *)&newt,(char *)&oldt, sizeof(struct termios));

  if (speed == 9600)
  {
    arg1 = 0x96;
    arg2 = 0x00;
    cfsetospeed(&newt, B9600);
    cfsetispeed(&newt, B9600);
  }
  else if (speed == 19200)
  {
    arg1 = 0x19;
    arg2 = 0x20;
    cfsetospeed(&newt, B19200);
    cfsetispeed(&newt, B19200);
  }
  else if (speed == 38400)
  {
    arg1 = 0x38;
    arg2 = 0x40;
    cfsetospeed(&newt, B38400);
    cfsetispeed(&newt, B38400);
  }
  else if (speed == 57600)
  {
    arg1 = 0x57;
    arg2 = 0x60;
    cfsetospeed(&newt, B57600);
    cfsetispeed(&newt, B57600);
  }
  else if (speed == 115200)
  {
    arg1 = 0x11;
    arg2 = 0x52;
    cfsetospeed(&newt, B115200);
    cfsetispeed(&newt, B115200);
  }
  else
  {
    fprintf(stderr,"speed not supported %d",speed);
    exit;
  }

  kodak_dc210_send_command(serialdev,DC_SET_SPEED,arg1,arg2,0x00,0x00);

  if (tcsetattr(serialdev, TCSANOW, &newt) < 0) 
    error_dialog("Serial speed change problem");
}

int kodak_dc210_command_complete (int serialdev)
{
  int status = DC_BUSY;

  /* Wait for the camera to be ready. */
  do
  {
    status = kodak_dc210_read_byte(serialdev);
  }
  while (status == DC_BUSY);

  if (status != DC_COMMAND_COMPLETE)
  {
    if (status == DC_ILLEGAL_PACKET)
    {
      fprintf(stderr,"kodak_dc210_command_complete - illegal packet received from host\n");
    }
    else
    {
      fprintf(stderr,"kodak_dc210_command_complete - command not completed\n");
    }
  }
}

unsigned char kodak_dc210_read_byte ( int serialdev )
{
  unsigned char b;
  int numread;

  do
  {
    numread = read(serialdev, &b, 1);
  } while (numread == 0);

  return(b);
}

int kodak_dc210_write_byte ( int serialdev, char b )
{
  int numwrite;

  numwrite = write(serialdev, &b, 1);

  if (numwrite != 1)
  {
    fprintf(stderr,"could not write byte\n");
  }
}

int kodak_dc210_read_packet ( int serialdev, char *packet, int length)
{
  int sent_checksum;
  int numread = 0;
  int r = 0;
  unsigned char computed_checksum;
  unsigned char control_byte;

  /* Read the control byte from the camera - and packet coming from
     the camera must ALWAYS be 0x01 according to Kodak Host Protocol. */
  control_byte = kodak_dc210_read_byte(serialdev);
  if (control_byte != PKT_CTRL_RECV)
  {
    fprintf(stderr,"kodak_dc210_read_packet - packet control byte invalid %x\n",control_byte);
  }

  do
  {
    r = read(serialdev,packet + numread,length-numread);

    if (r > 0)
    {
      numread += r;
    }
  }
  while ((r >= 0) && (numread < length));

  if (r < 0)
  {
    fprintf(stderr,"error reading packet %d, errno: %d\n",r,errno);
    exit;
  }
  else if (numread != length)
  {
    fprintf(stderr,"read incomplete packet %d of %d bytes\n",numread,length);
    exit;
  }

  sent_checksum = kodak_dc210_read_byte(serialdev);
  computed_checksum = kodak_dc210_checksum(packet,length);

  if (sent_checksum != computed_checksum)
  {
    fprintf(stderr,"bad checksum %d != %d\n",computed_checksum,sent_checksum);
    kodak_dc210_write_byte(serialdev,DC_ILLEGAL_PACKET);
  }
  else
  {
    kodak_dc210_write_byte(serialdev,DC_CORRECT_PACKET);
  }
}

unsigned char kodak_dc210_checksum(char *packet,int length)
{
/* Computes the *primitave* kodak checksum using XOR - yuck. */
  unsigned char chksum = 0;
  int i;

  for (i = 0 ; i < length ; i++)
  {
    chksum ^= packet[i];
  }

  return chksum;
}

int kodak_dc210_initialize() 
{
    return(1);
}

int kodak_dc210_open_camera (const char *devname)
{
  int serialdev;

  struct termios newt, oldt;

  serialdev = open(devname, O_RDWR|O_NOCTTY);

  if (serialdev < 0) 
    error_dialog("Cannot open device");

  if (tcgetattr(serialdev, &oldt) < 0) 
    error_dialog("tcgetattr");

  memcpy((char *)&newt,(char *)&oldt, sizeof(struct termios));

  /* need the device to be raw. 8 bits no parity on 9600 baud to start.  */
  cfmakeraw(&newt);
  newt.c_oflag &= ~CSTOPB;
  newt.c_cflag &= ~PARENB;
  newt.c_cflag |= CS8;
  newt.c_cflag &= ~PARODD;

  newt.c_cc[VMIN] = 0;
  newt.c_cc[VTIME] = 10;

  cfsetospeed(&newt, B9600);
  cfsetispeed(&newt, B9600);
  if (tcsetattr(serialdev, TCSANOW, &newt) < 0) 
    error_dialog("Serial speed change problem");

  kodak_dc210_send_command(serialdev,DC_INITIALIZE,0x00,0x00,0x00,0x00);
  kodak_dc210_command_complete(serialdev);
  kodak_dc210_set_port_speed(serialdev,115200);

  return serialdev;
}

int kodak_dc210_close_camera (int serialdev) 
{

  kodak_dc210_set_port_speed(serialdev,9600);
  close(serialdev);
  return(1);
}

int kodak_dc210_number_of_pictures () 
{

  int serialdev = kodak_dc210_open_camera (serial_port);
  struct kodak_dc210_status status;

  if (serialdev)
  {
    kodak_dc210_send_command( serialdev,DC_STATUS,0x00,0x00,0x00,0x00);
    kodak_dc210_read_packet( serialdev,(char *)&status,256);
    kodak_dc210_command_complete( serialdev );
    kodak_dc210_close_camera ( serialdev );
  }

  return(status.num_pictures);
}

int kodak_dc210_take_picture() 
{
  int serialdev;

  if (serialdev = kodak_dc210_open_camera (serial_port))
  {
    kodak_dc210_send_command( serialdev,DC_TAKE_PICTURE,0x00,0x00,0x00,0x00);
    kodak_dc210_command_complete( serialdev );
    kodak_dc210_close_camera ( serialdev );
  }

  return( kodak_dc210_number_of_pictures() );
}

struct Image *kodak_dc210_get_thumbnail (int serialdev, int picNum) 
{
  FILE *fh;
  struct Image *im = NULL;
  int i,j;
  int numRead = 0;
  int fileSize = 20736;
  int blockSize = 1024;
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

  /* the protocol expects 0..n-1, but this function is supplied 1..n */
  picNum--;

  /* allocate space for the thumbnail data */
  picData = (char *)malloc(fileSize + blockSize);

  /* allocate space for the image data */
  imData = (char *)malloc(fileSize + 54);

  kodak_dc210_send_command(serialdev,DC_PICTURE_THUMBNAIL,0x00,(char)picNum,0x01,0x00);
  while (numRead < fileSize)
  {
    kodak_dc210_read_packet( serialdev,picData+numRead,blockSize);
    numRead += blockSize;
    fprintf(stderr,"%d - %d of %d\n",(char)picNum,numRead,fileSize);
  }
  kodak_dc210_command_complete(serialdev);

  /* allocate memory for image structure */
  if ( (im = (struct Image *)malloc ( sizeof(struct Image) )) == NULL ) 
  {
    error_dialog("Could not allocate memory for image structure.");
    return ( NULL );
  }
  
  memcpy(imData,bmpHeader, sizeof(bmpHeader));

  /* reverse the thumbnail data */
  for (j=fileSize-1,i=54; j >= 0 ; j--)
  {
    imData[i++] = picData[j];
  }

  strcpy ( im->image_type, "bmp" );
  im->image_info = NULL;
  im->image_info_size = 0;
  im->image_size = fileSize+54;
  im->image = imData;

  free(picData);

  fh = fopen("/tmp/2.bmp","w");
  fwrite(imData,fileSize+54,1,fh);
  fclose(fh);

  return im;
}



struct Image *kodak_dc210_get_picture (int picNum, int thumbnail) 
{
  struct Image *im = NULL;
  int blockSize;
  int fileSize;
  int numRead;
  int serialdev;
  FILE *fh;
  struct kodak_dc210_status status;
  struct kodak_dc210_picture_info picInfo;
  char *picData;

  if (serialdev = kodak_dc210_open_camera (serial_port))
  {
    if (thumbnail)
    {
      im = kodak_dc210_get_thumbnail(serialdev,picNum);
    }
    else
    {
      /* DC210 addresses pictures 0..n-1 instead of 1..n */
      picNum--;

      kodak_dc210_send_command( serialdev,DC_STATUS,0x00,0x00,0x00,0x00);
      kodak_dc210_read_packet( serialdev,(char *)&status,256);
      kodak_dc210_command_complete( serialdev );

      /* send picture info command, receive data, send ack */
      kodak_dc210_send_command( serialdev,DC_PICTURE_INFO,0x00,(char)picNum,0x00,0x00);
      kodak_dc210_read_packet( serialdev,(char *)&picInfo,256);
      kodak_dc210_command_complete( serialdev );

      picInfo.fileSize = bswap_32(picInfo.fileSize);
      picInfo.elapsedTime = bswap_32(picInfo.elapsedTime);
      picInfo.pictureNumber = bswap_16(picInfo.pictureNumber);

      /* send the command to start transfering the picture */
      kodak_dc210_send_command( serialdev, DC_PICTURE_DOWNLOAD, 0x00, (char)picNum, 0x00, 0x00);

      fileSize = picInfo.fileSize;
      blockSize = 1024;
      picData = (char *)malloc(fileSize + blockSize);
      numRead = 0;

      while (numRead < fileSize)
      {
	kodak_dc210_read_packet( serialdev,picData+numRead,blockSize);
	numRead += blockSize;
	fprintf(stderr,"%d - %d of %d\n",(char)picNum,numRead,fileSize);
      }

/*      fh = fopen("/tmp/1.jpg","w");
      fwrite(picData,fileSize,1,fh);
      fclose(fh);*/
    
      kodak_dc210_command_complete( serialdev );

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

    /* close the camera */
    kodak_dc210_close_camera ( serialdev );
  }

  return(im);
}

int kodak_dc210_delete_picture (int picNum) 
{
  int serialdev;

  if (serialdev = kodak_dc210_open_camera (serial_port))
  {
    picNum--;

    kodak_dc210_send_command( serialdev,DC_ERASE_IMAGE_IN_CARD,0x00,(char)picNum,0x00,0x00);
    kodak_dc210_command_complete( serialdev );
    kodak_dc210_close_camera ( serialdev );
  }

  return (1);
}


struct Image *kodak_dc210_get_preview () 
{
  int numPicBefore;
  int numPicAfter;
  struct Image *im = NULL;
 
  /* find out how many pictures are in the camera so we can 
     make sure a picture was taken later */
  numPicBefore = kodak_dc210_number_of_pictures();

  /* take a picture -- it returns the picture number taken */
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

int kodak_dc210_configure () 
{

  error_dialog("Configuration is not supported through gPhoto, use the menus on the camera.");
  return(0);

}



char *kodak_dc210_summary() 
{

  char summary_string[2048];
  char buff[1024];
  int serialdev = kodak_dc210_open_camera (serial_port);
  FILE *fd;
  struct kodak_dc210_status status;

  if (serialdev)
  {
    kodak_dc210_send_command( serialdev,DC_STATUS,0x00,0x00,0x00,0x00);
    kodak_dc210_read_packet( serialdev,(char *)&status,256);
    kodak_dc210_command_complete( serialdev );
    kodak_dc210_close_camera ( serialdev );
  }

  strcpy(summary_string,"Kodak DC210\n");

  snprintf(buff,1024,"Camera Type: %d\n",status.camera_type_id);
  strcat(summary_string,buff);

  snprintf(buff,1024,"Firmware: %d.%d\n",status.firmware_major,status.firmware_minor);
  strcat(summary_string,buff);

  snprintf(buff,1024,"Battery Status: %d\n",status.batteryStatusId);
  strcat(summary_string,buff);

  snprintf(buff,1024,"AC Status: %d\n",status.acStatusId);
  strcat(summary_string,buff);

  snprintf(buff,1024,"Time: %d\n",status.time);
  strcat(summary_string,buff);

  snprintf(buff,1024,"Total Pictures Taken: %d\n",status.totalPicturesTaken);
  strcat(summary_string,buff);

  snprintf(buff,1024,"Total Flashes Fired: %d\n",status.totalFlashesFired);
  strcat(summary_string,buff);

  snprintf(buff,1024,"Pictures in Camera: %d\n",status.num_pictures);
  strcat(summary_string,buff);

  snprintf(buff,1024,"Camera Identification: %s\n",status.camera_ident);
  strcat(summary_string,buff);

  fd = fopen("/tmp/poop", "w+");
  fwrite(&status,sizeof(status),1,fd);
  fclose(fd);


/*struct kodak_dc210_status {*/
/*   short zoomMode;     	     	/* 17-18 */
/*   char flash_charged;		/* 20 */
/*   char compression_mode_id;	/* 21 */
/*  char flash_mode;		/* 22 */
/*   char exposure_compensation;	/* 23 */
/*   char picture_size;		/* 24 */
/*   char file_Type;		/* 25 */
/**/
/*  error_dialog("No summary.");*/
  return (&summary_string);
}

char *kodak_dc210_description() 
{

  return(
"Kodak DC210 support
by Brian Hirt <bhirt@berkhirt.com>.");
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
