/*
 * Copyright (C) 1999 by Henning Zabel <henning@uni-paderborn.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
 
/*
 * gphoto driver for the Mustek MDC800 Digital Camera. The driver 
 * supports rs232 and USB. It automatically detects which Kernelnode
 * is used.
 */
 #define _DEVICE_C
#include "device.h"
#undef _DEFICE_C

#include "print.h"
#include "../src/gphoto.h"

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <fcntl.h>
#include <string.h>


int mdc800_device_USB=0;

/*
 * Sets up the serial device and inits the baud rate.
 * If USB is set, setting of baudrate is ignored.
 */
int mdc800_device_setupDevice (fd, baud)
     int fd;
     int baud;
{
	struct termios newtio;

	if (mdc800_device_USB)
		return 1;

	bzero(&newtio, sizeof(newtio));

	newtio.c_cflag = (newtio.c_cflag & ~CSIZE) | CS8;

	//  Set into raw, no echo mode 
	newtio.c_iflag &= ~(IGNBRK | IGNCR | INLCR | ICRNL | IUCLC |
		IXANY | IXON | IXOFF | INPCK | ISTRIP);
	newtio.c_iflag |= (BRKINT | IGNPAR);
	newtio.c_oflag &= ~OPOST;
	newtio.c_lflag = ~(ICANON | ISIG | ECHO | ECHONL | ECHOE | ECHOK);
	newtio.c_cflag &= ~(CRTSCTS | PARENB | PARODD);
	newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cc[VMIN] = 1;
	newtio.c_cc[VTIME] = 0;

	
	cfsetispeed(&newtio, baud);
	cfsetospeed(&newtio, baud);
	
  	//tcflush(fd, TCIFLUSH);
	if (tcsetattr(fd,TCSANOW,&newtio) < 0)
		return 0;
	
	//fcntl(fd, F_SETFL, FASYNC);
	return 1;
}


/*
 * opens the specified device and inits it as a RS232
 * if USB is false. The Parameter USB is unused.
 */
int mdc800_device_open (path,USB)
     char *path;
	  int USB;
{

	int fd = open(path, O_RDWR | O_NOCTTY /*| O_SYNC | O_NONBLOCK*/);
	
	if (fd < 0)
	{
		printCError ("(mdc800_device_open) can't open device \"%s\".\n", path);
		return (-1);
	}
	
	mdc800_device_probeUSB (fd);

	if (!mdc800_device_setupDevice (fd,B57600))
		return -1;

		
	if (fcntl(fd,F_SETFL,0) < 0) 
		return -1;
   
	return fd;
}


/*
 * read Bytes from the device. If longtimeout is true,
 * the timeout is set to 1sec. It is needed for commands
 * (take_picture) taking much time.
 */
int mdc800_device_read(fd, p, c, longtimeout)
     int fd;
     char *p;
     int c;
	  int longtimeout;
{
   struct timeval Timeout;
	fd_set 			readfs;    // file descriptor set 
	int            readen=0;
	
	FD_ZERO(&readfs);
   FD_SET(fd, &readfs);
	
	while (readen < c)
	{
		// set timeout value within input loop 
		Timeout.tv_usec = MDC800_TTY_TIMEOUT_USEC;             // milliseconds 
		Timeout.tv_sec  = MDC800_TTY_TIMEOUT_SEC+longtimeout;  // seconds 
			
		select(fd+1 , &readfs, NULL, NULL, &Timeout);
		if (FD_ISSET (fd,&readfs))
		{
			int now=read (fd, p, c-readen);
			
			if (now < 0)
			{
				printCError ("(mdc800_device_read) tty read error (%i).\n",readen);
				return 0;
			}
			else
			{
				//(*p)=ch;
				//p++;
				//readen++;
				p+=now;
				readen+=now;
			}
		}
		else
		{
			printCError ("(mdc800_device_read) tty not responding (%i).\n",readen);
			return 0;
		}
	}
	return (readen);
}


/*
 * Flushes something
 */
void mdc800_device_flush(fd)
     int fd;
{
	tcflush(fd, TCOFLUSH);
}


/*
 * Writes the specified amount of Byte
 */
int mdc800_device_write (int fd, char* buf, int b)
{
	int r;
	mdc800_device_flush (fd);	
	r=write (fd, buf,b);
	return r;
}

/*
 * Closes the device
 */
int mdc800_device_close (int fd)
{
	return close (fd);
}


/*
 * Sets the baud rate 
 */
int mdc800_device_changespeed (int fd, int baud)
{
	struct termios newtio;

	if (mdc800_device_USB)
		return 1;

	printFnkCall ("(mdc800_device_changespeed) called\n");
	
	bzero(&newtio, sizeof(newtio));
	
	if (tcgetattr (fd , &newtio) < 0)
	{
		printCError ("(mdc800_device_changespeed) tcgetattr fails.\n");
		return 0;
	}
	
	cfsetispeed(&newtio, baud);
   cfsetospeed(&newtio, baud);
  
	if (tcsetattr(fd ,TCSAFLUSH,&newtio) < 0)
	{
		printCError ("(mdc800_device_changespeed) tcsetattr fails.\n");
		return 0;
	}
	
	return  1;
}


/*
 * Probe wether the used device is probably a USB device.
 * If we can't get device Preferences, we asume that we use
 * an USB device.
 */
int mdc800_device_probeUSB (int fd)
{
	struct termios tio;
	mdc800_device_USB=(tcgetattr (fd , &tio) < 0);
	return mdc800_device_USB;
}
 

/*
 * USB detected ?
 */
int 	mdc800_device_USB_detected ()
{
	return mdc800_device_USB;
}
