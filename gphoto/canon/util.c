/****************************************************************************
 *
 * File: util.c 
 *
 * Utility functions for the gPhoto Canon PowerShot A5(Z)/A50 driver.
 *
 ****************************************************************************/

/****************************************************************************
 *
 * include files
 *
 ****************************************************************************/

#include <stdio.h>

/*****************************************************************************
 *
 * dump_hex
 *
 * Dumps a memory area as hex on the screen.
 *
 * msg  - Info message for the dump
 * buf  - the memory buffer to dump
 * len  - length of the buffer
 *
 ****************************************************************************/

#define NIBBLE(_i)    (((_i) < 10) ? '0' + (_i) : 'A' + (_i) - 10)

void dump_hex(const char *msg, const unsigned char *buf, int len)
{
    int i;
    int nlocal;
    const unsigned char *pc;
    char *out;
    const unsigned char *start;
    char c;
    char line[100];

    start = buf;

#if 0
    if (len > 160)
    {
	printf("dump n:%d --> 160\n", len);
	len = 160;
    }
#endif

    printf("%s: (%d bytes)\n", msg, len);
    while (len > 0)
    {
	sprintf(line, "%08x: ", buf - start);
	out = line + 10;

	for (i = 0, pc = buf, nlocal = len; i < 16; i++, pc++)
	{
	    if (nlocal > 0)
	    {
		c = *pc;

		*out++ = NIBBLE((c >> 4) & 0xF);
		*out++ = NIBBLE(c & 0xF);

		nlocal--;
	    }
	    else
	    {
		*out++ = ' ';
		*out++ = ' ';
	    }			/* end else */

	    *out++ = ' ';
	}			/* end for */

	*out++ = '-';
	*out++ = ' ';

	for (i = 0, pc = buf, nlocal = len;
	     (i < 16) && (nlocal > 0);
	     i++, pc++, nlocal--)
	{
	    c = *pc;

	    if ((c < ' ') || (c >= 126))
	    {
		c = '.';
	    }

	    *out++ = c;
	}			/* end for */

	*out++ = 0;

	printf("%s\n", line);

	buf += 16;
	len -= 16;
    }				/* end while */
}				/* end dump */

/****************************************************************************
 *
 * End of file: util.c
 *
 ****************************************************************************/
