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




/***************************************************************
**
**
*/
int ConfigDSCF55Speed()
{
	int speed = B9600;
	char *spd_buf;

	spd_buf = getenv("DSCF55E_SPEED");

	if(spd_buf)
	{
		printf("%s\n", spd_buf);

		switch((char)*spd_buf)
		{
			case '1':
				speed = B115200;
				break;
			case '9':
			default:
				speed = B9600;
				break;
		}
	}

	fprintf(stderr, "Speed set to %u\n", speed);

	return speed;
}



/***************************************************************
**
**
*/
int ConfigDSCF55Port(char *serialbuf, int maxlen)
{
	char *port_buf = getenv("DSCF55E_PORT");

	if(!port_buf)
		return FALSE;

	strncpy(serialbuf, port_buf, maxlen);
	fprintf(stderr, "Port set to %s\n", serialbuf);

	return TRUE;
}



