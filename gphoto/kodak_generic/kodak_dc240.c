/*
 *      Copyright (C) 1999 Randy Scott <scottr@wwa.com>
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "kodak_generic.h"
#include "state_machine.h"
#include "kodak_command.h"
#include "kodak_dc240_helpers.h"

/*==============================================================================
* Preprocessor Defines
==============================================================================*/

#define HIGH_HPBS    8192

/*==============================================================================
* Type definitions
==============================================================================*/
typedef struct
{
   unsigned char filename[11];
   unsigned char attributes;
   unsigned short creation_time;
   unsigned short creation_date;
   unsigned char size[4];
} DIRECTORY_ENTRY_TYPE;

typedef struct
{
   unsigned char *filename;
   unsigned long thumbnail_size;
   unsigned long picture_size;
   unsigned char valid;
} PICTURE_TYPE;

typedef struct
{
   BOOLEAN valid;
   unsigned char data_type;
   unsigned char camera_type;
   unsigned short firmware_version;
   unsigned short rom32_version;
   unsigned short rom8_version;
   unsigned char battery_status;
   unsigned char adapter_flag;
   unsigned char strobe_status;
   unsigned char padding[256 - 11];
} STATUS_PACKET_TYPE;

typedef struct
{
   unsigned short num_entries;
   DIRECTORY_ENTRY_TYPE entries[1];
} DIRECTORY_PACKET_DATA_TYPE;

typedef struct
{
   BOOLEAN valid;
   unsigned char *filename;
   int rx_bytes;
   DIRECTORY_PACKET_DATA_TYPE *rx_buffer;
} DIRECTORY_PACKET_TYPE;

typedef struct
{
   BOOLEAN valid;
   unsigned char *filename;
   int size;
   int rx_bytes;
   unsigned char *rx_buffer;
} PICTURE_PACKET_TYPE;

typedef struct
{
   BOOLEAN valid;
   unsigned char *filename;
   unsigned char data[256];
} PICTURE_INFO_PACKET_TYPE;

/*==============================================================================
* Local Function Prototypes
==============================================================================*/
static void kdc240_post_initialize(STATE_MACHINE_INSTANCE *);

static int kdc240_detect(void);
static int kdc240_initialize(void);

static struct Image *kdc240_get_picture(int, int);
static unsigned char *kdc240_get_picture_tx_filename (PICTURE_PACKET_TYPE *);
static CC_RESPONSE_TYPE kdc240_get_picture_read (PICTURE_PACKET_TYPE *,
   unsigned char *);
static void kdc240_get_picture_info(PICTURE_TYPE *);
static unsigned char *kdc240_get_picture_info_tx_filename (
   PICTURE_INFO_PACKET_TYPE *);
static CC_RESPONSE_TYPE kdc240_get_picture_info_read(
   PICTURE_INFO_PACKET_TYPE *, unsigned char *);

static struct Image *kdc240_get_preview(void);
static int kdc240_delete_picture(int);
static int kdc240_take_picture(void);

static int kdc240_number_of_pictures(void);
static unsigned char *kdc240_number_of_pictures_tx_filename (
   DIRECTORY_PACKET_TYPE *);
static CC_RESPONSE_TYPE kdc240_number_of_pictures_read (
   DIRECTORY_PACKET_TYPE *, unsigned char *);

static int kdc240_configure(void);

static char *kdc240_summary(void);
static CC_RESPONSE_TYPE kdc240_summary_read(STATUS_PACKET_TYPE *,
   unsigned char *);

static char *kdc240_description(void);

static void kdc240_open_card(void);
static void kdc240_close_card(void);
static void kdc240_set_hpbs(void);

static unsigned char *kdc240_create_filename(unsigned char *,
   unsigned long, unsigned long);

/*==============================================================================
* Global Variables
==============================================================================*/
STATE_MACHINE_INSTANCE *machine;
static PICTURE_TYPE *picture_index = NULL;

/*==============================================================================
* Constants
==============================================================================*/
STATE_MACHINE_TEMPLATE template =
{
   serial_port,
   9600,
   kdc240_post_initialize
};

struct Kodak_Camera kodak_dc240 =
{
   kdc240_detect,
   kdc240_initialize,
   kdc240_get_picture,
   kdc240_get_preview,
   kdc240_delete_picture,
   kdc240_take_picture,
   kdc240_number_of_pictures,
   kdc240_configure,
   kdc240_summary,
   kdc240_description
};

/*==============================================================================
* Local Functions
==============================================================================*/
/*******************************************************************************
* FUNCTION: kdc240_post_intialize
*
* DESCRIPTION:
*
*    This function is called after the state machine has powered up.  It is
*    responsible for ensuring that the camera is ready to talk with the
*    driver.
*
*******************************************************************************/
static void
kdc240_post_initialize
(
   STATE_MACHINE_INSTANCE *mach
)
{
   /* Store the machine instance */
   machine = mach;

   /* Fire up the camera */
   kdc240_restart();
}

/*******************************************************************************
* FUNCTION: kdc240_detect
*
* DESCRIPTION:
*
*    Auto-detects the presence of the DC240.  Returns 1 if successfully
*    detected the camera, 0 otherwise.
*
*******************************************************************************/
static int
kdc240_detect
(
   void
)
{
   /* No detection yet */
   return 1;
}

/*******************************************************************************
* FUNCTION: kdc240_initialize
*
* DESCRIPTION:
*******************************************************************************/
static int
kdc240_initialize
(
   void
)
{
   machine = state_machine_construct(&template);
   if (machine == NULL)
   {
      printf ("kdc240_initialize: could not create state machine!\n");
      return 0;
   }

   return 1;
}

/*******************************************************************************
* FUNCTION: kdc240_get_picture
*
* DESCRIPTION:
*******************************************************************************/
static struct Image *
kdc240_get_picture
(
   int picture,
   int thumbnail
)
{
   struct Image *image = NULL;
   PICTURE_PACKET_TYPE buffer;
   CC_STRUCT_TYPE cc_struct = 
   {
      (int)&buffer,
      58,  (void *)kdc240_get_picture_tx_filename,
      HIGH_HPBS, (void *)kdc240_get_picture_read
   };

   /* gphoto uses a one-based index, turn it into a zero-based one. */
   picture--;

   kdc240_open_card();

   /* Get picture information if we haven't already */
   if (!picture_index[picture].valid)
   {
      kdc240_get_picture_info(&picture_index[picture]);
      if (!picture_index[picture].valid)
      {
         kdc240_close_card();
         return NULL;
      }
   }

   buffer.valid = FALSE;
   buffer.rx_bytes = 0;
   buffer.rx_buffer = NULL;
   buffer.filename = kdc240_create_filename(picture_index[picture].filename,
      0, 0);

   kdc240_set_hpbs();

   if (thumbnail)
   {
      buffer.size = picture_index[picture].thumbnail_size;
      kdc240_complex_command(&cc_struct, KODAK_CAMERA_DC240,
         CMD_READ_THUMBNAIL, 2);
   }
   else
   {
      buffer.size = picture_index[picture].picture_size;
      kdc240_complex_command(&cc_struct, KODAK_CAMERA_DC240,
         CMD_READ_FILE);
   }

   kdc240_close_card();

   if (buffer.valid)
   {
      image = (struct Image *)malloc(sizeof(struct Image));
      image->image_size = buffer.size;
      image->image = buffer.rx_buffer;
      strcpy(image->image_type, "jpg");
      image->image_info_size = 0;
   }
   else if (buffer.rx_buffer)
   {
      free(buffer.rx_buffer);
   }

   return image;
}

static unsigned char *
kdc240_get_picture_tx_filename
(
   PICTURE_PACKET_TYPE *buffer
)
{
   return buffer->filename;
}

static CC_RESPONSE_TYPE
kdc240_get_picture_read
(
   PICTURE_PACKET_TYPE *buffer,
   unsigned char *packet
)
{
   int bytes_to_copy;
   CC_RESPONSE_TYPE retval;

   /* Handle receive errors */
   if (packet == NULL)
   {
      return CC_RESPONSE_LAST_PACKET;
   }

   if (buffer->rx_bytes == 0)
   {
      buffer->rx_buffer = (unsigned char *)malloc(buffer->size);
   }

   bytes_to_copy = buffer->size - buffer->rx_bytes;

   if (bytes_to_copy > HIGH_HPBS)
   {
      bytes_to_copy = HIGH_HPBS;
      retval = CC_RESPONSE_MORE_PACKETS;
   }
   else
   {
      buffer->valid = TRUE;
      retval = CC_RESPONSE_LAST_PACKET;
   }

   memcpy(buffer->rx_buffer + buffer->rx_bytes, packet, bytes_to_copy);
   buffer->rx_bytes += bytes_to_copy;

   printf ("kdc240_get_picture_read: read %d bytes\n", buffer->rx_bytes);

   return retval;
}

static void
kdc240_get_picture_info
(
   PICTURE_TYPE *picture
)         
{     
   int size;
   PICTURE_INFO_PACKET_TYPE buffer;
   CC_STRUCT_TYPE cc_struct =
   {
      (int)&buffer,
      58,  (void *)kdc240_get_picture_info_tx_filename,
      256, (void *)kdc240_get_picture_info_read,
   };

   printf ("kdc240_get_picture_info: filename %s\n", picture->filename);

   buffer.valid = FALSE;
   buffer.filename = kdc240_create_filename(picture->filename, 0, 0);
   
   kdc240_complex_command(&cc_struct, KODAK_CAMERA_DC240, CMD_READ_PIC_INFO);

   if (buffer.valid)
   {
      size = (buffer.data[92] << 24) |
             (buffer.data[93] << 16) |
             (buffer.data[94] <<  8) |
             (buffer.data[95] <<  0);
      picture->thumbnail_size = size;

      size = (buffer.data[104] << 24) |
             (buffer.data[105] << 16) |
             (buffer.data[106] <<  8) |
             (buffer.data[107] <<  0);
      picture->picture_size = size;

      picture->valid = TRUE;

      printf ("        thumbnail size: %ld\n", picture->thumbnail_size);
      printf ("        picture size:   %ld\n", picture->picture_size);
   }
}

static unsigned char *
kdc240_get_picture_info_tx_filename
(  
   PICTURE_INFO_PACKET_TYPE *buffer
)  
{  
   return buffer->filename;
}     
         
static CC_RESPONSE_TYPE
kdc240_get_picture_info_read
(  
   PICTURE_INFO_PACKET_TYPE *buffer,
   unsigned char *packet
)     
{
   if (packet != NULL)
   {
      buffer->valid = TRUE;
      memcpy(buffer->data, packet, 256);
   }
      
   return CC_RESPONSE_LAST_PACKET;
}

/*******************************************************************************
* FUNCTION: kdc240_get_preview
*
* DESCRIPTION:
*******************************************************************************/
static struct Image *
kdc240_get_preview
(
   void
)
{
}

/*******************************************************************************
* FUNCTION: kdc240_delete_picture
*
* DESCRIPTION:
*******************************************************************************/
static int
kdc240_delete_picture
(
   int picture
)
{
}

/*******************************************************************************
* FUNCTION: kdc240_take_picture
*
* DESCRIPTION:
*******************************************************************************/
static int
kdc240_take_picture
(
   void
)
{
}

/*******************************************************************************
* FUNCTION: kdc240_number_of_pictures
*
* DESCRIPTION:
*******************************************************************************/
static int
kdc240_number_of_pictures
(
   void
)
{
   int retval;
   int i;

   DIRECTORY_PACKET_TYPE buffer;
   CC_STRUCT_TYPE cc_struct = 
   {
      (int)&buffer,
      58,  (void *)kdc240_number_of_pictures_tx_filename,
      256, (void *)kdc240_number_of_pictures_read
   };

   kdc240_open_card();

   buffer.valid = FALSE;
   buffer.filename = kdc240_create_filename("\\DCIM\\100DC240\\*.JPG", 0, 0);
   buffer.rx_bytes = 0;
   buffer.rx_buffer = NULL;

   kdc240_complex_command(&cc_struct, KODAK_CAMERA_DC240, CMD_READ_DIRECTORY);

   free(buffer.filename);

   kdc240_close_card();

   retval = 0;
   if (buffer.valid)
   {
      if (picture_index)
      {
         free(picture_index);
      }

      picture_index = (PICTURE_TYPE *)malloc(
         sizeof(PICTURE_TYPE) * buffer.rx_buffer->num_entries);

      if (picture_index == NULL)
      {
         printf ("kdc240_number_of_pictures: picture_index == NULL\n");
      }
      else
      {
         retval = buffer.rx_buffer->num_entries;
         for (i = 0; i < retval; i++)
         {
             PICTURE_TYPE *t = &picture_index[i];

             t->valid = FALSE;
             t->filename = (unsigned char *)malloc(128);
             memset(t->filename, 0, 128);
             strcpy (t->filename, "\\DCIM\\100DC240\\");
             memcpy (t->filename + strlen(t->filename),
                buffer.rx_buffer->entries[i].filename, 8);
             t->filename[strlen(t->filename)] = '.';
             memcpy (t->filename + strlen(t->filename),
                buffer.rx_buffer->entries[i].filename + 8, 3);

             printf ("    Entry %d: %s\n", i, t->filename);
         }
      }
   }

   if (buffer.rx_buffer)
   {
      free(buffer.rx_buffer);
   }

   return retval;
}

static unsigned char *
kdc240_number_of_pictures_tx_filename
(
   DIRECTORY_PACKET_TYPE *buffer
)
{
   return buffer->filename;
}

static CC_RESPONSE_TYPE
kdc240_number_of_pictures_read
(
   DIRECTORY_PACKET_TYPE *buffer,
   unsigned char *packet
)
{
   int num_entries;
   int bytes_to_copy;
   CC_RESPONSE_TYPE retval;

   /* Handle receive errors */
   if (packet == NULL)
   {
      return CC_RESPONSE_LAST_PACKET;
   }

   if (buffer->rx_bytes == 0)
   {
      num_entries = (packet[0] << 8) | packet[1];
printf ("kdc240_number_of_pictures_read: first. num_entries = %d\n", num_entries);
      buffer->rx_buffer = (DIRECTORY_PACKET_DATA_TYPE *)
         malloc((sizeof(DIRECTORY_ENTRY_TYPE) * (num_entries << 1)) + 2);
printf ("kdc240_number_of_pictures_read: buffer = %p\n", buffer->rx_buffer);
   }
   else
   {
printf ("kdc240_number_of_pictures_read: other. rx_bytes = %d\n", buffer->rx_bytes);
      num_entries = buffer->rx_buffer->num_entries;
   }

   bytes_to_copy = (sizeof(DIRECTORY_ENTRY_TYPE) * num_entries) + 2;
printf ("kdc240_number_of_pictures_read: total size %d\n", bytes_to_copy);
   bytes_to_copy -= buffer->rx_bytes;
printf ("kdc240_number_of_pictures_read: left %d\n", bytes_to_copy);

   if (bytes_to_copy > 256)
   {
      bytes_to_copy = 256;
      retval = CC_RESPONSE_MORE_PACKETS;
printf ("kdc240_number_of_pictures_read: 256 byte packet\n");
   }
   else
   {
      buffer->valid = TRUE;
      retval = CC_RESPONSE_LAST_PACKET;
printf ("kdc240_number_of_pictures_read: last packet = %d\n", bytes_to_copy);
   }

printf ("kdc240_number_of_pictures_read: buffer = %p\n", buffer->rx_buffer);
printf ("kdc240_number_of_pictures_read: packet = %p\n", packet);
   memcpy(buffer->rx_buffer + buffer->rx_bytes, packet, bytes_to_copy);
   buffer->rx_bytes += bytes_to_copy;
printf ("kdc240_number_of_pictures_read: new rx_bytes = %d\n", buffer->rx_bytes);
   buffer->rx_buffer->num_entries = ntohs(buffer->rx_buffer->num_entries);

   return retval;
}

/*******************************************************************************
* FUNCTION: kdc240_configure
*
* DESCRIPTION:
*******************************************************************************/
static int
kdc240_configure
(
   void
)
{
   return 0;
}

/*******************************************************************************
* FUNCTION: kdc240_summary
*
* DESCRIPTION:
*******************************************************************************/
static char *
kdc240_summary
(
   void
)
{
   STATUS_PACKET_TYPE buffer = { FALSE };
   CC_STRUCT_TYPE cc_struct = 
   {
      (int)&buffer,
      0, NULL,
      256, (void *)kdc240_summary_read,
   };

   kdc240_complex_command(&cc_struct, KODAK_CAMERA_DC240, CMD_STATUS);

   if (buffer.valid)
   {
      unsigned char format_buf[100];
      unsigned char *summary_buf = (char *)calloc(4096, sizeof(unsigned char));

      strcat (summary_buf, "Camera is a Kodak Digital Science ");
      switch (buffer.camera_type)
      {
         case 3:
             strcat (summary_buf, "DC200\n");
             break;

         case 4:
             strcat (summary_buf, "DC210\n");
             break;

         case 5:
             strcat (summary_buf, "DC240\n");
             break;

         default:
             strcat (summary_buf, "(unknown)\n");
             break;
      }

      strcat (summary_buf,"Firmware version: ");
      sprintf(format_buf, "0x%04X\n", buffer.firmware_version);
      strcat (summary_buf, format_buf);

      strcat (summary_buf,"Battery status: ");
      switch (buffer.battery_status)
      {
          case 0:
              strcat (summary_buf, "OK\n");
              break;

          case 1:
              strcat (summary_buf, "Weak\n");
              break;

          case 2:
              strcat (summary_buf, "Empty\n");
              break;

          default:
              strcat (summary_buf, "(error)\n");
              break;
      }

      strcat (summary_buf,"AC Adapter status: ");
      switch (buffer.adapter_flag)
      {
          case 0:
              strcat (summary_buf, "not connected\n");
              break;

          case 1:
              strcat (summary_buf, "connected\n");
              break;

          default:
              strcat (summary_buf, "(error)\n");
              break;
      }

      return summary_buf;
   }
   else
   {
      return "Camera Error";
   }
}

static CC_RESPONSE_TYPE
kdc240_summary_read
(
   STATUS_PACKET_TYPE *buffer,
   unsigned char *packet
)
{
   if (packet != NULL)
   {
      buffer->valid = TRUE;
      memcpy(&(buffer->data_type), packet, 256);
   }

   return CC_RESPONSE_LAST_PACKET;
}

/*******************************************************************************
* FUNCTION: kdc240_description
*
* DESCRIPTION:
*******************************************************************************/
static char *
kdc240_description
(
   void
)
{
   return "Kodak DC240.  Copyright (c) 1999, Randy Scott <scottr@wwa.com>";
}

static void
kdc240_open_card
(
   void
)
{
   kdc240_command(KODAK_CAMERA_DC240, CMD_OPEN_CARD);
}

static void
kdc240_close_card
(
   void
)
{
   kdc240_command(KODAK_CAMERA_DC240, CMD_CLOSE_CARD);
}

static void
kdc240_set_hpbs
(
   void
)
{
   kdc240_command(KODAK_CAMERA_DC240, CMD_SET_HPBS, HIGH_HPBS + 2);
}

static unsigned char *
kdc240_create_filename
(
   unsigned char *filename,
   unsigned long start,
   unsigned long number
)
{
   unsigned char *buf; 
 
   buf = (unsigned char *)malloc(58);
   memset(buf, 0, 58);

   strcpy (buf, filename);

   buf[48] = (start >> 24) & 0xff;
   buf[49] = (start >> 16) & 0xff;
   buf[50] = (start >>  8) & 0xff;
   buf[51] = (start >>  0) & 0xff;

   buf[52] = (number >> 24) & 0xff;
   buf[53] = (number >> 16) & 0xff;
   buf[54] = (number >>  8) & 0xff;
   buf[55] = (number >>  0) & 0xff;

   return buf;
}

/*==============================================================================
* Global Functions
==============================================================================*/
/*******************************************************************************
* FUNCTION: kdc240_register
*
* DESCRIPTION:
*
*    Registers the camera model's existance with the Kodak generic handler.
*    Expects to be called when the generic handler is initialized.
*
*******************************************************************************/
void
kdc240_register
(
   void
)
{
   kodak_register(&kodak_dc240);
}
