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

/*==============================================================================
* Include files
==============================================================================*/
#include <stdio.h>
#include <stdlib.h>

#include "state_machine.h"
#include "kodak_generic.h"
#include "kodak_command.h"
#include "kodak_dc240.h"
#include "kodak_dc240_helpers.h"

/*==============================================================================
* Preprocessor defines
==============================================================================*/
#define TRANSMIT_COMMAND(desc) \
  { (desc), 8, 1, kdc240_get_command, kdc240_read_ack, kdc240_read_ack_error }

#define COMMAND_COMPLETE(desc) \
  { (desc), 0, 1, NULL, kdc240_read_cmd_complete, kdc240_read_cmd_complete_error }

#define HIGH_BAUD_RATE      115200
#define HIGH_BAUD_RATE_CODE 0x1152

/*==============================================================================
* Types
==============================================================================*/

/*==============================================================================
* Local Variables
==============================================================================*/
static BOOLEAN camera_init = FALSE;

/*==============================================================================
* Local Function Prototypes
==============================================================================*/
void kdc240_baud_command(void);

unsigned char * kdc240_get_command (int);
NEXT_STATE_TYPE kdc240_read_ack (int, unsigned char *);
NEXT_STATE_TYPE kdc240_read_ack_error (int);
NEXT_STATE_TYPE kdc240_read_cmd_complete (int, unsigned char *);
NEXT_STATE_TYPE kdc240_read_cmd_complete_error (int);

/*==============================================================================
* Local Functions
==============================================================================*/
/*******************************************************************************
* FUNCTION: kdc240_get_command
*
* DESCRIPTION:
*******************************************************************************/
unsigned char *
kdc240_get_command
(
   int descriptor
)
{
   return kodak_command_get(descriptor);
}

/*******************************************************************************
* FUNCTION: kdc240_read_ack
*
* DESCRIPTION:
*******************************************************************************/
NEXT_STATE_TYPE
kdc240_read_ack
(
   int descriptor,
   unsigned char *data
)
{
   if (*data == COMMAND_RESPONSE_ACK)
   {
      return STATE_NEXT;
   }
   else
   {
      printf ("kdc240_read_ack: got a 0x%02X\n", *data);
      camera_init = FALSE;
      return STATE_ABORT;
   }
}

/*******************************************************************************
* FUNCTION: kdc240_read_ack_error
*
* DESCRIPTION:
*******************************************************************************/
NEXT_STATE_TYPE
kdc240_read_ack_error
(
   int descriptor
)
{
   printf ("kdc240_read_ack_error: read error\n");
   camera_init = FALSE;
   return STATE_ABORT;
}

/*******************************************************************************
* FUNCTION: kdc240_read_cmd_complete
*
* DESCRIPTION:
*******************************************************************************/
NEXT_STATE_TYPE
kdc240_read_cmd_complete
(
   int descriptor,
   unsigned char *data
)
{
   if (*data == COMMAND_RESPONSE_COMPLETE)
   {
      return STATE_NEXT;
   }
   else if (*data == COMMAND_RESPONSE_BUSY)
   {
      printf ("kdc_read_cmd_complete: busy\n");
      return STATE_REPEAT;
   }
   else
   {
      printf ("kdc_read_cmd_complete: got a 0x%02X\n", *data);
      camera_init = FALSE;
      return STATE_ABORT;
   }
}

/*******************************************************************************
* FUNCTION: kdc240_read_cmd_complete_error
*
* DESCRIPTION:
*******************************************************************************/
NEXT_STATE_TYPE
kdc240_read_cmd_complete_error
(
   int descriptor
)
{
   printf ("kdc240_read_cmd_complete_error: read error\n");
   camera_init = FALSE;
   return STATE_ABORT;
}

NEXT_STATE_TYPE
kdc240_baud_switch
(
   int descriptor,
   unsigned char *packet
)
{
   printf ("kdc240_baud_switch: switching to 115200\n");

   /* Pause after changing baud rate */
   sleep(1);

   /* Set the new baud rate */
   state_machine_set_baud(machine, HIGH_BAUD_RATE);

   /* Indicate camera is initialized */
   camera_init = TRUE;

   /* Pause again */
   sleep(1);

   return STATE_NEXT;
}

NEXT_STATE_TYPE
kdc240_read_packet_control
(
   CC_STRUCT_TYPE *cc_struct,
   unsigned char *packet
)
{
   if (*packet != 0x01)
   {
      printf ("kdc240_read_packet_control: bad control 0x%02X\n", *packet);
      cc_struct->rx_func(cc_struct->descriptor, NULL);
      camera_init = FALSE;
      return STATE_ABORT;
   }

   return STATE_NEXT;
}

NEXT_STATE_TYPE
kdc240_read_packet
(
   CC_STRUCT_TYPE *cc_struct,
   unsigned char *packet
)
{
   CC_RESPONSE_TYPE resp;
   int i;
   int checksum;

   checksum = 0;
   for (i = 0; i < cc_struct->rx_packet_size; i++)
   {
      checksum ^= packet[i];
   }

   if (checksum != packet[i])
   {
      printf ("kdc240_read_packet: checksum error\n");
      cc_struct->packet_response = COMMAND_RESPONSE_ILL_PACKET;
      cc_struct->rx_func(cc_struct->descriptor, NULL);
   }
   else
   {
      cc_struct->packet_response = COMMAND_RESPONSE_CORRECT;
      cc_struct->resp = cc_struct->rx_func(cc_struct->descriptor, packet);
   }

   return STATE_NEXT;
}

NEXT_STATE_TYPE
kdc240_read_packet_error
(
   CC_STRUCT_TYPE *cc_struct
)
{
   printf ("kdc240_read_packet_error: read error\n");
   cc_struct->rx_func(cc_struct->descriptor, NULL);
   camera_init = FALSE;
   return STATE_ABORT;
}

unsigned char *
kdc240_send_packet_response
(
   CC_STRUCT_TYPE *cc_struct
)
{
   return &(cc_struct->packet_response);
}

NEXT_STATE_TYPE
kdc240_packet_response_done
(
   CC_STRUCT_TYPE *cc_struct,
   unsigned char *packet
)
{
   if (cc_struct->packet_response == COMMAND_RESPONSE_ILL_PACKET)
   {
      return cc_struct->loop_top;
   }
   else if (cc_struct->resp == CC_RESPONSE_LAST_PACKET)
   {
      return STATE_NEXT;
   }
   else
   {
      return cc_struct->loop_top;
   }
}

static unsigned char *
kdc240_send_packet_control
(
   CC_STRUCT_TYPE *cc_struct
)
{
   cc_struct->packet_response = 0x80;
   return &(cc_struct->packet_response);
}

static NEXT_STATE_TYPE
kdc240_packet_control_done
(
   CC_STRUCT_TYPE *cc_struct,
   unsigned char *packet
)
{
   return STATE_NEXT;
}

static unsigned char *
kdc240_send_packet
(
   CC_STRUCT_TYPE *cc_struct
)
{
   unsigned char *data;
   unsigned char *buffer;
   int checksum;
   int i;

   buffer = (unsigned char *)malloc(cc_struct->tx_packet_size + 1);
   data = cc_struct->tx_func(cc_struct->descriptor);

   checksum = 0;
   for (i = 0; i < cc_struct->tx_packet_size; i++)
   {
      buffer[i] = data[i];
      checksum ^= data[i];
   }

   buffer[i] = checksum;

   cc_struct->tx_buffer = buffer;

   return buffer;
}

static NEXT_STATE_TYPE
kdc240_read_packet_resp
(
   CC_STRUCT_TYPE *cc_struct,
   unsigned char *packet
)
{
   free(cc_struct->tx_buffer);

   if (*packet == COMMAND_RESPONSE_CORRECT)
   {
      return STATE_NEXT;
   }
   else
   {
      printf ("kdc240_read_packet_resp: bad response 0x%02X\n", *packet);
      return STATE_1;
   }
}

static NEXT_STATE_TYPE
kdc240_read_packet_resp_error
(
   CC_STRUCT_TYPE *cc_struct,
   unsigned char *packet
)
{
   free(cc_struct->tx_buffer);

   printf ("kdc240_read_packet_resp_error: aborting\n");
   return STATE_ABORT;
}

/*==============================================================================
* Global Functions
==============================================================================*/
/*******************************************************************************
* FUNCTION: kdc240_restart
*
* DESCRIPTION:
*******************************************************************************/
void
kdc240_restart
(
   void
)  
{  
   /* Send break */
   state_machine_assert_break(machine);
   
   /* Switch the camera into high gear */
   kdc240_baud_command();
}

/*******************************************************************************
* FUNCTION: kdc240_command
*
* DESCRIPTION:
*
*    Creates and sends a simple command.  Simple commands are defined as those
*    involving no data packets sent between the computer and camera.
*
*******************************************************************************/
void
kdc240_command
(
   KODAK_CAMERA_TYPE camera,
   unsigned char command,
   ...
)
{
   int desc;
   va_list ap;

   if (!camera_init)
   {
      kdc240_restart();
      if (!camera_init)
      {
         printf ("kdc240_command: Is your camera turned on?\n");
         return;
      }
   }

   va_start(ap, command);
   desc = kodak_command_vcreate(camera, command, ap);
   va_end(ap);

   {
      STATE_MACHINE_LINE lines[2] = {
         TRANSMIT_COMMAND(desc),
         COMMAND_COMPLETE(desc)
      };
      STATE_MACHINE_PROGRAM program = { 2, lines };

      state_machine_program(machine, &program);
      while (state_machine_run(machine))
      {
         /* Loop until done */
      }
   }
}

/*******************************************************************************
* FUNCTION: kdc240_complex_command
*
* DESCRIPTION:
*
*    Creates and sends a complex command.
*
*******************************************************************************/
void
kdc240_complex_command
(
   CC_STRUCT_TYPE *cc_struct,
   KODAK_CAMERA_TYPE camera,
   unsigned char command,
   ...
)
{
   int desc;
   va_list ap;

   if (!camera_init)
   {
      kdc240_restart();
      if (!camera_init)
      {
         printf ("kdc240_command: Is your camera turned on?\n");
         return;
      }
   }

   va_start(ap, command);
   desc = kodak_command_vcreate(camera, command, ap);
   va_end(ap);

   if (cc_struct->tx_func == NULL)
   {
      STATE_MACHINE_LINE lines[] = {
         TRANSMIT_COMMAND(desc),
         { (int)cc_struct, 0, 1, NULL,
            (void *)kdc240_read_packet_control,
            (void *)kdc240_read_packet_error },
         { (int)cc_struct, 0, cc_struct->rx_packet_size + 1, NULL,
            (void *)kdc240_read_packet, (void *)kdc240_read_packet_error },
         { (int)cc_struct, 1, 0, (void *)kdc240_send_packet_response,
            (void *)kdc240_packet_response_done, NULL },
         COMMAND_COMPLETE(desc)
      };
      STATE_MACHINE_PROGRAM program = { 5, lines };

      cc_struct->loop_top = STATE_1;

      state_machine_program(machine, &program);
      while (state_machine_run(machine))
      {
         /* Loop until done */
      }
   }
   else
   {
      STATE_MACHINE_LINE lines[] = {
         TRANSMIT_COMMAND(desc),
         { (int)cc_struct, 1, 0, (void *)kdc240_send_packet_control,
            (void *)kdc240_packet_control_done, NULL },
         { (int)cc_struct, cc_struct->tx_packet_size + 1, 1,
            (void *)kdc240_send_packet, (void *)kdc240_read_packet_resp,
            (void *)kdc240_read_packet_resp_error },
         { (int)cc_struct, 0, 1, NULL,
            (void *)kdc240_read_packet_control,
            (void *)kdc240_read_packet_error },
         { (int)cc_struct, 0, cc_struct->rx_packet_size + 1, NULL,
            (void *)kdc240_read_packet, (void *)kdc240_read_packet_error },
         { (int)cc_struct, 1, 0, (void *)kdc240_send_packet_response,
            (void *)kdc240_packet_response_done, NULL },
         COMMAND_COMPLETE(desc)
      };
      STATE_MACHINE_PROGRAM program = { 7, lines };

      cc_struct->loop_top = STATE_3;

      state_machine_program(machine, &program);
      while (state_machine_run(machine))
      {
         /* Loop until done */
      }
   }
}

/*******************************************************************************
* FUNCTION: kdc240_baud_command
*
* DESCRIPTION:
*
*
*******************************************************************************/
void
kdc240_baud_command
(
   void
)
{
   int desc;
   va_list ap;

   desc = kodak_command_create(KODAK_CAMERA_DC240, CMD_BAUD_RATE,
      HIGH_BAUD_RATE_CODE);

   {
      STATE_MACHINE_LINE lines[] = {
         TRANSMIT_COMMAND(desc),
         { desc, 0, 0, NULL, kdc240_baud_switch, NULL } 
      };
      STATE_MACHINE_PROGRAM program = { 2, lines };

      state_machine_program(machine, &program);
      while (state_machine_run(machine))
      {
         /* Loop until done */
      }
   }

   /* Pause after changing baud rate */
   sleep(1);

   /* Set the new baud rate */
   state_machine_set_baud(machine, HIGH_BAUD_RATE);
}
