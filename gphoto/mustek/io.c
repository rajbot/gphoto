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
 
/*
 * Implements cummunication-core :
 *
 * Sending commands and receiving data
 */
#define _IO_C
#include "io.h"
#undef _IO_C
#include "../src/gphoto.h"

#include <fcntl.h>
#include <termios.h>
#include <sys/time.h>
#include "device.h"
#include "print.h"
#include "rs232.h"
#include "usb.h"
#include "mdc800_spec.h"

//--------------------- Communicating to the Camera --------------------------/
           
/*
 * Opens Connection to Camera, 
 * flag: 0: use RS232 , 1:USB (unused)
 * and sends initial Packet.
 */
int mdc800_io_openDevice (char* device,int flag)
{
	if (mdc800_device_handle != -1)
	{
		printCError ("(mdc800_io_openDevice) Camera was already open.\n");
		return 1;
		//closeCamera ();
	}
	mdc800_device_handle=mdc800_device_open (device,flag);
	
	if (mdc800_device_handle == -1)
	{
		printCError ("(mdc800_io_openDevice) can't open camera.\n");
		return 0;
	}
	
	return 1;
}


/*
 * Closes the connection to the camera
 */
int mdc800_io_closeDevice ()
{
	if (mdc800_device_handle == -1)
	{
		printCError ("(mdc800_io_closeCamera) Camera is already closed !\n");
		return (1);
	}
	mdc800_device_close (mdc800_device_handle);
	mdc800_device_handle=-1;
	return (1);
}


/*
 * Sets the baud rate 
 */
int mdc800_io_changespeed (int baud)
{
	return mdc800_device_changespeed (mdc800_device_handle, baud);
}



/*
 * sends a command and receives the answer to this
 * buffer: Strores the answer
 * length: length of answer or null
 */
int mdc800_io_sendCommand (char commandid,char par1,char par2,char par3, char* buffer, int length)
{
	char command [8];
	command[0]=COMMAND_BEGIN;
	command[1]=commandid;
	command[2]=par1;
	command[3]=par2;
	command[4]=par3;
	command[5]=COMMAND_END;
	command[6]=0;
	command[7]=0;
	if (mdc800_device_USB_detected ())
		return mdc800_usb_sendCommand (command, buffer, length );
	else
		return mdc800_rs232_sendCommand ( command, buffer, length);
}


/*
 * Send an USB Command if USB is detected
 */
int mdc800_io_sendUSBCommand (char commandid , char b1, char b2, char b3, char b4, char b5, char b6, char* buffer, int length)
{
	char command [8];
	
	if (!mdc800_device_USB_detected ())
	{
		printAPIError ("USB is not detected\n");
		return 0;
	}
	
	printCError ("(mdc800_io_sendUSBCommand) id:%i (%i,%i,%i,%i,%i,%i) answer:%i\n",(unsigned char) commandid,(unsigned char) b1,(unsigned char) b2,(unsigned char) b3,(unsigned char) b4,(unsigned char) b5,(unsigned char) b6, length);
	command[0]=COMMAND_BEGIN;
	command[1]=commandid;
	command[2]=b1;
	command[3]=b2;
	command[4]=b3;
	command[5]=b4;
	command[6]=b5;
	command[7]=b6;
	return mdc800_usb_sendCommand ( command, buffer, length);	
}
