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
	
	Revision 2.7  1998/10/18 13:18:27  crosser
	Put RCS logs and I.D. into the source

	Revision 2.6  1998/02/26 00:50:39  crosser
	misc changes
	
	Revision 2.5  1998/02/13 23:02:40  crosser
	define type off_t for DOS
	
	Revision 2.4  1998/02/08 19:58:38  crosser
	Support low memory: chunked saving etc.
	
	Revision 2.3  1998/02/03 18:47:51  lightner
	Fix typo: definded -> defined
	
	Revision 2.2  1998/01/18 02:16:45  crosser
	DOS support
	
	Revision 2.1  1998/01/04 13:55:57  crosser
	add param for close mode
	
	Revision 2.0  1998/01/02 19:20:11  crosser
	Added support for Win32
	
	Revision 1.1  1997/08/17 08:59:54  crosser
	Initial revision
	
*/

#ifndef _EPH_IO_H
#define _EPH_IO_H

#include <sys/types.h>
#ifdef DOS
typedef long off_t;
#endif
#if defined(UNIX)
#include <termios.h>
#elif defined(MSWINDOWS)
#include <windows.h>
#endif
#include <stdlib.h>

#ifndef DC1
#define DC1 0x11
#endif

#define MAX_SPEED 115200

typedef struct _eph_iob {
	void (*errorcb)(int errcode,char *errstr);
	void *(*realloccb)(void *old,size_t length);
	void (*runcb)(off_t count);
	int (*storecb)(char *data,size_t size);
	int debug;
#if defined(UNIX)
	int fd;
	struct termios savetios;
#elif defined(MSWINDOWS)
	HANDLE fd;
	DCB savedcb;
	COMMTIMEOUTS savetimeouts,worktimeouts;
#elif defined(DOS)
	int fd;
#endif
	unsigned long timeout;
} eph_iob;

eph_iob *eph_new(void (*errorcb)(int errcode,char *errstr),
		void *(*realloccb)(void *old,size_t length),
		void (*runcb)(off_t count),
		int (*storecb)(char *data,size_t size),
		int debug);
int eph_open(eph_iob *iob,char *device_name,long speed);
int eph_close(eph_iob *iob,int newmodel);
void eph_free(eph_iob *iob);

int eph_setint(eph_iob *iob,int reg,long val);
int eph_getint(eph_iob *iob,int reg,long *val);
int eph_action(eph_iob *iob,int reg,char *val,size_t length);
int eph_setvar(eph_iob *iob,int reg,char *val,off_t length);
int eph_getvar(eph_iob *iob,int reg,char **val,off_t *length);

#define ERR_BASE		10001
#define ERR_DATA_TOO_LONG	10001
#define ERR_TIMEOUT		10002
#define ERR_BADREAD		10003
#define ERR_BADDATA		10004
#define ERR_BADCRC		10005
#define ERR_BADSPEED		10006
#define ERR_NOMEM		10007
#define ERR_BADARGS		10008
#define ERR_EXCESSIVE_RETRY	10009
#define ERR_MAX			10010

#define REG_FRAME		4
#define REG_SPEED		17
#define REG_IMG			14
#define REG_TMN			15

#endif
