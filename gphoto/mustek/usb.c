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
 * supports rs232 and USB. It automatically detects which Kernlnode
 * is used.
 */

/*
	Implemenation of the USB Version of ExecuteCommand
	
*/
#include "../src/gphoto.h"
#include "print.h"
#include "usb.h"
#include "device.h"
#include "io.h"
#include "mdc800_spec.h"

int mdc800_usb_sendCommand (char* command, char* buffer, int length)
{
	int leaveout=0, toread=0,blocksize=0;
	char p[348160];

	printFnkCall ("(mdc800_usb_sendCommand) id:%i (%i,%i,%i,%i,%i,%i),answer:%i\n",command[1],command[2],command[3],command[4],command[5],command[6],command[7],length);		
	if (mdc800_device_handle == -1)
	{
		printCError ("(mdc800_usb_sendCommand) Camera is not open !\n");
		return 0;
	}
	
	/* Send the Command */
	if (mdc800_device_write (mdc800_device_handle, command ,8) != 8)
	{
		printCError ("(mdc800_usb_sendCommand) sending Command fails!\n");
		return 0;
	}

	/* receive the answer */
	switch ((unsigned char) command [1])
	{
		case COMMAND_GET_THUMBNAIL:
		case COMMAND_GET_IMAGE:
			leaveout=64;
			blocksize=64;
			toread=length;
			break;
	
		default :
			leaveout=8;
			toread=8;
			blocksize=16;
			if (length == 0)
				toread=0;
	}
	
	
	if (toread)
	{
		/* Now read the answer */
		int i=0;
		
		for (i=0; i*blocksize<toread+leaveout; i++)
		{
			if (mdc800_device_read (mdc800_device_handle,&p[i*blocksize],blocksize,0) != blocksize)
			{
				printCError ("(mdc800_usb_sendCommand) receiving answer fails.\n");
				return 0;
			}
			if (blocksize > 16)
				update_progress ((float) i*blocksize/(toread+leaveout));
		}

		/* Copy answer to buffer */
		for (i=0; i<length; i++)
			buffer[i]=p[leaveout+i];	

	}
	return 1;
}
