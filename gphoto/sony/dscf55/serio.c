/*


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
    http://www.gnu.org/copyleft/gpl.html

 */

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#include "serio.h"

static int dscf55_fd;
static struct	termios local;
static struct	termios master;




/***************************************************************
*
*
*/
int InitSonyDSCF55(char *devicename)
{
	char	buffer[256];

	dscf55_fd = open(devicename, O_RDWR|O_NOCTTY);

	if(-1 != dscf55_fd)
	{
		if(tcgetattr(dscf55_fd, &master)== -1)
		{
			perror("tcgetattr failed\n");
			return FALSE;
		}

		memcpy(&local, &master, sizeof(struct termios));
		cfmakeraw(&local);

		local.c_cc[VMIN] = 0;
		local.c_cc[VTIME] = 5;

		if(!dscSetSpeed(B9600))
		{
			perror("dscSetSpeed");
		}

		while(read(dscf55_fd, buffer, 256)>0);

		return TRUE;
	}

	printf("return with FALSE\n");

	return FALSE;
}


/***************************************************************
*
*
*/
void CloseSonyDSCF55()
{
	if(-1 != dscf55_fd)
	{
		close(dscf55_fd);
	}
}


/***************************************************************
*
*
*/
int dscSetSpeed(int speed)
{
	usleep(50000);

	switch(speed)
	{
		case B115200:
			cfsetispeed(&local, B115200);
			cfsetospeed(&local, B115200);
			break;
		default:
			cfsetispeed(&local, B9600);
			cfsetospeed(&local, B9600);
			break;
	}

	return !tcsetattr(dscf55_fd, TCSANOW, &local);
}


/***************************************************************
*
*
*/
int Read(unsigned char *buffer, int *length)
{
	int len = read(dscf55_fd, buffer, *length);

	if(len < 0)
	{
		perror("Read failed\n");
		*length = 0 ;
	}

	return len;
}


/***************************************************************
*
*
*/
int ReadCommByte(unsigned char *byte)
{
	int len = read(dscf55_fd, byte, 1);

	if(len < 0)
		perror("Read failed\n");

	return len;
}


/***************************************************************
*
*
*/
int Write(unsigned char *buffer, int length)
{
	int bytecount;

	for(bytecount=0; bytecount<length; bytecount++)
	{
/*
		if(length==2)
			printf("{%u}",(unsigned char)*(buffer+bytecount));
*/

		if(write(dscf55_fd, buffer+bytecount, 1) != 1)
		{
			printf("Write failed\n");
			break;
		}
	}

	return bytecount;
}


/***************************************************************
*
*
*/
void DumpPacket(Packet *p)
{
	DumpData(p->buffer, p->length);
}


/***************************************************************
*
*
*/
void DumpData(unsigned char *buffer, int length)
{
	int n=0;

	printf("Dumping :");

	for(n=0; n<length; n++)
	{
		printf("%u ", (int) ((unsigned char )buffer[n]));
	}

	fflush(stdout);
}


