/*
 * Copyright (c) 1999, Randy Scott <scottr@wwa.com>.  All rights reserved.
 */

#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <errno.h>

#include "state_machine.h"

#define BAUDCASE(x) case (x): { ret = B##x; break; }

static int baudconv(int);

/******************************************************************************
* FUNCTION: state_machine_construct
*
* DESCRIPTION:
******************************************************************************/
STATE_MACHINE_INSTANCE *
state_machine_construct
(
   STATE_MACHINE_TEMPLATE *template
)
{
   STATE_MACHINE_INSTANCE *self;

   self = (STATE_MACHINE_INSTANCE *)malloc(sizeof(STATE_MACHINE_INSTANCE));
   if (self == NULL)
   {
printf ("malloc failed in state machine construct\n");
      return NULL;
   }

   self->fd = open(template->device, O_RDWR|O_NDELAY);
   if (self->fd == -1)
   {
      perror("state_machine_construct: open");
      free(self);
      return NULL;
   }

   state_machine_set_baud(self, template->baud_rate);

   template->driver_init(self);

   return self;
}

/******************************************************************************
* FUNCTION: state_machine_destruct
*
* DESCRIPTION:
******************************************************************************/
void
state_machine_destruct
(
   STATE_MACHINE_INSTANCE *self
)
{
   if (self == NULL)
   {
      return;
   }

   close(self->fd);

   if (self->rx_buffer)
   {
      free(self->rx_buffer);
   }

   if (self->states)
   {
      free(self->states);
   }

   free(self);

   return;
}

/******************************************************************************
* FUNCTION: state_machine_set_baud
*
* DESCRIPTION:
******************************************************************************/
void
state_machine_set_baud
(
   STATE_MACHINE_INSTANCE *self,
   int baud
)
{
   struct termios t;

   if (tcgetattr(self->fd, &t) < 0)
   {
      perror("state_machine_set_baud: tcgetattr");
      return;
   }

   t.c_iflag = 0;
   t.c_oflag = 0;
   t.c_cflag = CS8 | CREAD | CLOCAL;
   t.c_lflag = 0;
   t.c_cc[VMIN] = 1;
   t.c_cc[VTIME] = 5;

   cfsetispeed(&t, baudconv(baud));
   cfsetospeed(&t, baudconv(baud));

   if (tcsetattr(self->fd, TCSANOW, &t) < 0)
   {
      perror("state_machine_set_baud: tcsetattr");
      return;
   }

   self->baud = baud;

   return;
}

/******************************************************************************
* FUNCTION: state_machine_program
*
* DESCRIPTION:
******************************************************************************/
void
state_machine_program
(
   STATE_MACHINE_INSTANCE *self,
   STATE_MACHINE_PROGRAM *program
)
{
   self->current = 0;
   self->num_states = program->num_states;
   self->states = (STATE_MACHINE_LINE *)malloc(
      sizeof(STATE_MACHINE_LINE) * self->num_states);

   if (self->states == NULL || self->num_states == 0)
   {
      free(self->states);
      self->states = NULL;
      self->num_states = 0;
      return;
   }

   memcpy (self->states, program->states,
      sizeof(STATE_MACHINE_LINE) * self->num_states);

   self->num_tx = 0;
   self->num_rx = 0;
   self->rx_buffer = NULL;

   return;
}

/******************************************************************************
* FUNCTION: state_machine_run
*
* DESCRIPTION:
*
*    State machine driver.  Returns TRUE when it needs to be called again,
*    FALSE otherwise.
*
******************************************************************************/
BOOLEAN
state_machine_run
(
   STATE_MACHINE_INSTANCE *self
)
{
   STATE_MACHINE_LINE *line;
   NEXT_STATE_TYPE next_state = STATE_INVALID;
   BOOLEAN retval = TRUE;

   if (self == NULL ||
       self->states == NULL ||
       self->current >= self->num_states)
   {
      return FALSE;
   }

   line = &self->states[self->current];

   /* Transmit sub-state */
   if (self->num_tx < line->bytes_write)
   {
      unsigned char *buffer = line->write_data(line->descriptor);
      int written;

#if 0
printf ("state_machine: tx: %d/%d\n",self->num_tx, line->bytes_write);
#endif

      /* Empty the read queue before writing */
      tcflush(self->fd, TCIOFLUSH);

      written = write(self->fd, buffer + self->num_tx,
         (line->bytes_write - self->num_tx));

      if (written == -1)
      {
         perror("state_machine_run: write");
         next_state = line->error_handler(line->descriptor);
      }
      else
      {
         self->num_tx += written;
         if (self->num_tx < line->bytes_write)
         {
            next_state = STATE_REPEAT;
         }
         else
         {
            /* Ensure that all data was actually written */
            tcdrain(self->fd);
            self->rx_buffer = NULL;
         }
      }
   }

   /* Receive sub-state */
   if (next_state == STATE_INVALID &&
       self->num_rx < line->bytes_read)
   {
      fd_set readfds;
      struct timeval tm;
      int ret;
      int num;

      FD_ZERO(&readfds);
      FD_SET(self->fd, &readfds);
      tm.tv_sec = 2;
      tm.tv_usec = 0;

#if 0
printf ("state_machine: rx: %d/%d\n",self->num_rx, line->bytes_read);
#endif

      ret = select (self->fd + 1, &readfds, NULL, NULL, &tm);
      if (ret <= 0)
      {
         if (ret < 0)
         {
            if (errno != EINTR)
            {
               perror("state_machine_run: select");
               next_state = line->error_handler(line->descriptor);
            }
            else
            {
               next_state = STATE_AGAIN;
            }
         }
         else
         {
            printf("state_machine_run: select: timeout\n");
            next_state = line->error_handler(line->descriptor);
         }
      }
      else
      {
         if (self->rx_buffer == NULL)
         {
            self->rx_buffer = (unsigned char *)malloc(line->bytes_read);
         }

         num = read (self->fd, self->rx_buffer + self->num_rx,
            (line->bytes_read - self->num_rx));

         if (num == -1)
         {
            perror("state_machine_run: read");
            next_state = line->error_handler(line->descriptor);
         }
         else
         {
            self->num_rx += num;
            if (self->num_rx < line->bytes_read)
            {
               next_state = STATE_AGAIN;
            }
         }
      }
   }

   if (next_state == STATE_INVALID)
   {
      next_state = line->read_data(line->descriptor, self->rx_buffer);
      free(self->rx_buffer);
      self->rx_buffer = NULL;
   }

   switch (next_state)
   {
      case STATE_NEXT:
         self->current++;
         self->num_tx = 0;
         self->num_rx = 0;
         retval = TRUE;
         break;

      case STATE_AGAIN:
         retval = TRUE;
         break;

      case STATE_REPEAT:
         self->num_tx = 0;
         self->num_rx = 0;
         retval = TRUE;
         break;

      case STATE_0:
      case STATE_1:
      case STATE_2:
      case STATE_3:
      case STATE_4:
      case STATE_5:
      case STATE_6:
         self->current = next_state - STATE_0;
         self->num_tx = 0;
         self->num_rx = 0;
         retval = TRUE;
         break;

      case STATE_ABORT:
         if (self->rx_buffer)
         {
            free(self->rx_buffer);
         }
         retval = FALSE;
         break;

      default:
         retval = FALSE;
   }

   return retval;
}

/******************************************************************************
* FUNCTION: state_machine_assert_break
*
* DESCRIPTION:
******************************************************************************/
void
state_machine_assert_break
(
   STATE_MACHINE_INSTANCE *self
)
{
   /* Assert break for between 0.25 and 0.5 seconds */
   tcsendbreak(self->fd, 0);

   /* Pause to allow camera to recover */
   sleep(1);

   return;
}

/******************************************************************************
* FUNCTION: baudconv
*
* DESCRIPTION:
******************************************************************************/
static int
baudconv
(
   int baud
)
{ 
   speed_t ret;
 
   ret = (speed_t) baud;
   switch (baud) {
      /* POSIX defined baudrates */
      BAUDCASE(0); 
      BAUDCASE(50);
      BAUDCASE(75);
      BAUDCASE(110);
      BAUDCASE(134);
      BAUDCASE(150);
      BAUDCASE(200);
      BAUDCASE(300);
      BAUDCASE(600);
      BAUDCASE(1200);
      BAUDCASE(1800);
      BAUDCASE(2400);
      BAUDCASE(4800);
      BAUDCASE(9600);
      BAUDCASE(19200);
      BAUDCASE(38400);
  
      /* non POSIX values */
#ifdef B7200
      BAUDCASE(7200); 
#endif
#ifdef B14400
      BAUDCASE(14400);
#endif
#ifdef B28800
      BAUDCASE(28800);
#endif
#ifdef B57600
      BAUDCASE(57600);
#endif
#ifdef B115200
      BAUDCASE(115200);
#endif
#ifdef B230400
      BAUDCASE(230400);
#endif
    
      default: 
         fprintf(stderr, "baudconv: baudrate %d is undefined; using as is\n", baud);
   }

   return ret; 
}
