/*
	$Id$
*/

/*
	Copyright (c) 1997,1998 Eugene G. Crosser
	Copyright (c) 1998 Bruce D. Lightner (DOS/Windows support)

	You may distribute and/or use for any purpose modified or unmodified
	copies of this software if you preserve the copyright notice above.

	THIS SOFTWARE IS PROVIDED AS IS AND COME WITH NO WARRANTY OF ANY
	KIND, EITHER EXPRESSED OR IMPLIED.  IN NO EVENT WILL THE
	COPYRIGHT HOLDER BE LIABLE FOR ANY DAMAGES RESULTING FROM THE
	USE OF THIS SOFTWARE.
*/

/*
	$Log$
	Revision 1.1  1999/05/27 18:32:06  scottf
	Initial revision

	Revision 1.1.1.1  1999/01/07 15:04:02  del
	Imported 0.2 sources
	
	Revision 2.2  1998/10/18 13:18:27  crosser
	Put RCS logs and I.D. into the source

	Revision 2.1  1998/01/18 02:16:45  crosser
	DOS support
	
	Revision 2.0  1998/01/02 19:20:11  crosser
	Added support for Win32
	
*/

#ifndef _USLEEP_H
#define _USLEEP_H

void usleep(long usecs);

#ifdef DOS
extern double delay_factor;
void start_time(void);
unsigned long elasped_usecs(void);
#endif

#endif
