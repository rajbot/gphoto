#include <string.h>
#include "kodak_generic.h"

/*
 *      Portions Copyright (C) 1999 Brent D. Metz <bmetz@vt.edu>
 *      Portions Copyright (C) 1998 Beat Christen <spiff@longstreet.ch>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published 
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* 
  It may say generic on the filename, but right now this isn't very generic :)
*/

void dump(char *str, char *buf, int len) 
{  
  printf("%s [ ", str);

  if(len>0) 
  {
    int p = 1;

    printf("0x%x", buf[0]);

    while(p<len)
      printf(", 0x%x", buf[p++]);
  }
  printf(" ]\n");
}


int             kodak_generic_initialize()
{
  return 1;
}
	
GdkImlibImage * kodak_generic_get_picture(int a, int b)
{
  return NULL;
}

GdkImlibImage * kodak_generic_get_preview()
{
  return NULL;
}

int             kodak_generic_delete_picture(int a)
{
  return 0;
}

int             kodak_generic_take_picture()
{
  return 0;
}

int             kodak_generic_number_of_pictures()
{
  return 0;
}

int             kodak_generic_configure()
{
  return 0;
}

io_link * kodak_generic_open(char *device)
{
    io_link *dev;
    char buf[20];
    int retries;
        
    dev = io_open("/dev/ttyS1");

    io_set_baudrate(dev, 9600);

    buf[0] = 0x72;
    buf[1] = 0x00;
    buf[2] = 0xFF;
    buf[3] = 0x00;
    buf[4] = 0x00;
    buf[5] = 0x00;
    buf[6] = 0x00;
    buf[7] = 0x1A;
    dump("send: ", buf, 8);
    io_sendbuffer(dev, buf, 8);

    io_recvbuffer(dev, 1);      
    dump("recv: ", dev->buf, dev->buflen);
    for (retries=0;(retries<3) && (dev->buf[0] != (char)0xD1);retries++) {
      printf("Retrying..\n");
      dev->buflen=0;
      io_sendbuffer(dev, buf, 8);
      io_recvbuffer(dev, 1);
    }
    if (retries == 3)
      return NULL;
    dev->buflen=0;
    io_recvbuffer(dev, 1);
    printf("Got here\n");
    io_recvbuffer(dev, 1);
    dump("Buf recv: ", dev->buf, dev->buflen);

    return dev; 
}

char *          kodak_generic_summary()
{
  io_link *dev;
  char buf[8];
  char format_buf[100];
  unsigned char *summary_buf;
  unsigned char checksum;
  int i;

  summary_buf = (char *)calloc(4096, sizeof(unsigned char));

  if ((dev = kodak_generic_open("/dev/ttyS1")) == NULL) {
    return NULL;
  }

  for (i=0;i<8;i++) {
    buf[i] = 0;
  }

  buf[0] = 0x7F;
  buf[7] = 0x1A;

  io_sendbuffer(dev, buf, 8);
  dev->buflen=0;
  io_recvbuffer(dev, 258);
  dump("recv: ", dev->buf, dev->buflen);

  if (dev->buflen != 259)
    printf("Strange buffer length %d\n", dev->buflen);

  checksum = 0;
  for (i=2;i<(dev->buflen-1);i++) {
    checksum ^= dev->buf[i]; 
  }

  if (checksum != dev->buf[258]) {
    printf("Checksum error!\n");
    buf[0] = 0xE3;
    io_sendbuffer(dev, buf, 1);
    dev->buflen=0;
    io_recvbuffer(dev, 258);
    dump("recv: ", dev->buf, dev->buflen);
    checksum = 0;
    for (i=2;i<(dev->buflen-1);i++) {
      checksum ^= dev->buf[i];
    }
  }

  strcat (summary_buf, "Camera is a Kodak Digital Science DC");
  strcat (summary_buf, ((dev->buf[3] == 3)?"200":"210"));
  strcat (summary_buf, "\n");
  
  strcat (summary_buf,"Firmware version: ");
  sprintf(format_buf, "%d.%s%d\n", dev->buf[4], ((dev->buf[5] < 10)?"0":""), dev->buf[5]);
  strcat (summary_buf, format_buf);

  strcat (summary_buf, "Power Source: ");
  if (dev->buf[11] == 0)
    strcat (summary_buf, "Batteries");
  else
    strcat (summary_buf, "AC Adapter");
  strcat(summary_buf, "\n");

  strcat (summary_buf, "Compression: ");
  if (dev->buf[21] == 1)
    strcat (summary_buf, "High");
  else if (dev->buf[21] == 2)
    strcat (summary_buf, "Medium");
  else
    strcat (summary_buf, "Low");
  strcat(summary_buf, "\n");

  strcat(summary_buf, "Compression Method: ");
  if (dev->buf[25] == 3)
    strcat(summary_buf, "JPEG");
  else if (dev->buf[25] == 4)
    strcat(summary_buf, "FlashPix");
  strcat(summary_buf, "\n");

  strcat (summary_buf, "Flash Mode: ");
  switch (dev->buf[22]) {
    case 0:    strcat(summary_buf, "Auto");
               break;
    case 1:    strcat(summary_buf, "Fill");
               break;
    case 2:    strcat(summary_buf, "Off");
               break;
    case 3:    strcat(summary_buf, "Auto Red-Eye");
               break;
    case 4:    strcat(summary_buf, "Fill Red-Eye");
               break;
  }
  strcat(summary_buf, "\n");

  strcat(summary_buf, "Resolution: ");
  if (dev->buf[24] == 0)
    strcat(summary_buf, "640x480");
  else if (dev->buf[24] == 1)
    strcat(summary_buf, "1152x864"); 
  strcat(summary_buf, "\n");

  strcat(summary_buf, "Total # of Pictures Ever Taken: ");
  sprintf(format_buf, "%d\n", ((unsigned char)dev->buf[27] << 8) | ((unsigned char)dev->buf[28]));
  strcat(summary_buf, format_buf); 

  strcat(summary_buf, "Total # of Flashes Fired: ");
  sprintf(format_buf, "%d\n", ((unsigned char)dev->buf[29] << 8) | ((unsigned char)dev->buf[30]));
  strcat(summary_buf, format_buf);

  strcat(summary_buf, "Number of Pictures in Camera: ");
  sprintf(format_buf, "%d\n", ((unsigned char)dev->buf[58] << 8) | ((unsigned char)dev->buf[59]));
  strcat(summary_buf, format_buf);

  strcat(summary_buf, "Remaining Pictures at High Compression: ");
  sprintf(format_buf, "%d\n", ((unsigned char)dev->buf[70] << 8) | ((unsigned char)dev->buf[71]));
  strcat(summary_buf, format_buf);

  strcat(summary_buf, "Remaining Pictures at Medium Compression: ");  
  sprintf(format_buf, "%d\n", ((unsigned char)dev->buf[72] << 8) | ((unsigned char)dev->buf[73]));
  strcat(summary_buf, format_buf);

  strcat(summary_buf, "Remaining Pictures at Low Compression: ");  
  sprintf(format_buf, "%d\n", ((unsigned char)dev->buf[74] << 8) | ((unsigned char)dev->buf[75]));
  strcat(summary_buf, format_buf);

  strcat(summary_buf, "Camera ID: ");
  for (i=0;i<32;i++)
    format_buf[i] = dev->buf[i+92];
  format_buf[32] = 0;
  strcat(summary_buf, format_buf);
  strcat(summary_buf, "\n");

  return summary_buf;
  
  /*strdup("This driver is not ready");*/
}

char *          kodak_generic_description()
{
  return strdup("This is the Generic Kodak driver");
}
