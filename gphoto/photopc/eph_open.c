
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

	Revision 1.2  1999/04/30 07:14:14  scottf
	minor changes to remove compilation warnings. prepping for release.
	
	Revision 1.1.1.1  1999/01/07 15:04:02  del
	Imported 0.2 sources
	
	Revision 2.14  1998/10/18 13:18:27  crosser
	Put RCS logs and I.D. into the source

	Revision 2.13  1998/08/08 14:00:33  crosser
	make switchoff friendly to Olympus
	
	Revision 2.12  1998/08/01 13:12:41  lightner
	Four ports for DOS
	
	Revision 2.11  1998/08/01 12:30:50  crosser
	flushinput function
	
	Revision 2.10  1998/05/09 18:21:08  crosser
	change O_NONBLOCK to O_NDELAY (is it correct?  I don't know.  On most
	systems they are the same, but what if they are not?  Tell me if you
	have problems.)
	
	Revision 2.9  1998/04/09 07:38:44  crosser
	fix semantics of the "switch off" arg
	
	Revision 2.8  1998/02/26 00:50:39  crosser
	change c_breake code for compatibility
	
	Revision 2.7  1998/02/25 22:47:24  crosser
	fix saving termios data
	
	Revision 2.6  1998/02/16 06:15:05  lightner
	Call _cexit() in ^C handler
	
	Revision 2.5  1998/01/27 21:52:55  crosser
	clean up error message, remove unneeded speep
	
	Revision 2.4  1998/01/18 02:16:45  crosser
	DOS support
	
	Revision 2.3  1998/01/05 19:47:49  lightner
	Win32 syntax error fixed: text->commtext
	
	Revision 2.2  1998/01/04 13:55:57  crosser
	add param for close mode
	
	Revision 2.1  1998/01/03 19:57:13  crosser
	Fix Windows things, improve error reporting
	
	Revision 2.0  1998/01/02 19:20:11  crosser
	Added support for Win32
	
	Revision 1.4  1997/12/24 00:19:13  crosser
	Change default speed to 115200
	
	Revision 1.3  1997/11/03 23:25:11  crosser
	add immediate session finish command
	
	Revision 1.2  1997/09/12 09:42:35  crosser
	skip possible NULs prior to `camera signature'
	
	Revision 1.1  1997/08/17 08:59:54  crosser
	Initial revision
	
*/

#include "eph_io.h"
#include "eph_priv.h"
#include <sys/types.h>
#if defined(MSWINDOWS)
#include "usleep.h"
#elif defined(UNIX)
#include <unistd.h>
#include <termios.h>
#elif defined(DOS)
#include "comio.h"
#include "usleep.h"
#else
#error platform not defined
#endif
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#ifdef MSWINDOWS
#define ERRNO GetLastError()
#else
#define ERRNO errno
#endif

#if defined(MSWINDOWS)

#define DEFSPEED 19200
#define CLOSE CloseHandle

#elif defined(UNIX)

#define DEFSPEED EXTA
#define CLOSE close

#elif defined(DOS)

#define DEFSPEED 19200
#define CLOSE(x)  ttclose()

void exit_cleanup(void) {
	ttclose();
}

#define ABORT 0

int c_break(void) {
	printf("\naborting program...%c\n", 7);
	ttclose();
	exit(255);	/* will call any exit handlers */
	/* notreached */
}

#else
#error platform not defined
#endif

int eph_open(eph_iob *iob,char *devname,long speed) {
#if defined(MSWINDOWS)
	DCB dcb = { 0 };
	char commtext[80];
#elif defined(UNIX)
	struct termios tios;
	speed_t tspeed;
#elif defined(DOS)
	int port;
#else
#error platform not defined
#endif
	long ephspeed;
	int rc;
	int count=0;

	if (speed == 0) speed=MAX_SPEED;

	switch (speed) {
	case 9600:
#ifdef UNIX
		tspeed=B9600;
#endif
		ephspeed=1;	break;
	case 19200:
#ifdef UNIX
		tspeed=EXTA;
#endif
		ephspeed=2;	break;
	case 38400:
#ifdef UNIX
		tspeed=EXTB;
#endif
		ephspeed=3;	break;
	case 57600:
#ifdef UNIX
#ifdef B57600
		tspeed=B57600;
#else
		eph_error(iob,ERR_BADSPEED,"specified speed %ld unsupported",
									speed);
		return -1;
#endif
#endif
		ephspeed=4;	break;
	case 115200:
#ifdef UNIX
#ifdef B115200
		tspeed=B115200;
#else
		eph_error(iob,ERR_BADSPEED,"specified speed %ld unsupported",
									speed);
		return -1;
#endif
#endif
		ephspeed=5;	break;
	default:
		eph_error(iob,ERR_BADSPEED,"specified speed %ld invalid",speed);
		return -1;
	}

	iob->timeout=DATATIMEOUT+((2048000000L)/speed)*10;
	if (iob->debug) printf("set timeout to %lu\n",DATATIMEOUT+iob->timeout);

#if defined(DOS)
	if (strcasecmp(devname,"COM1:") == 0) {
		port = 1;
	} else if (strcasecmp(devname,"COM2:") == 0) {
		port = 2;
	} else if (strcasecmp(devname,"COM3:") == 0) {
		port = 3;
	} else if (strcasecmp(devname,"COM4:") == 0) {
		port = 4;
	} else {
		eph_error(iob,ERRNO,"device name %s not COM1:, COM2:, COM3: or COM4:",
					devname);
		return -1;
	}
	ctrlbrk(c_break);
	atexit(exit_cleanup);
	usleep(1);
	if (iob->debug)
		printf("delay factor: %g loops/usec\n",(float)delay_factor);
	TTinit(port, DEFSPEED);
	if (ttopen()) {
		errno=2;
#elif defined(MSWINDOWS)
	if ((iob->fd=CreateFile(devname,
				GENERIC_READ | GENERIC_WRITE,
				0,
				0,
				OPEN_EXISTING,
				0,
				0)) == INVALID_HANDLE_VALUE) {
#elif defined(UNIX)
#ifndef O_NDELAY
#define O_NDELAY O_NONBLOCK
#endif
	if ((iob->fd=open(devname,O_RDWR | O_NDELAY)) < 0) {
#else
#error platform not defined
#endif
/*		if (strlen(devname) < 400)
eph_error(iob,ERRNO,"open %s error %s",devname,strerror(ERRNO));
*/
		return -1;
	}

#if defined(MSWINDOWS)

	if (!GetCommState(iob->fd, &dcb)) {
		eph_error(iob,ERRNO,"GetCommState error %s",strerror(ERRNO));
		CLOSE(iob->fd);
		return -1;
	}

	memcpy(&iob->savedcb,&dcb,sizeof(dcb));
	memset((char*)&dcb,0,sizeof(dcb));
	dcb.DCBlength = sizeof(dcb);
	sprintf(commtext, "%d,n,8,1", DEFSPEED);
	if (!BuildCommDCB(commtext, &dcb)) {
		eph_error(iob,ERRNO,"BuildCommDCB error %s",strerror(ERRNO));
		CLOSE(iob->fd);
		return -1;
	}
	if (!SetCommState(iob->fd, &dcb)) {
		eph_error(iob,ERRNO,"SetCommState error %s",strerror(ERRNO));
		CLOSE(iob->fd);
		return -1;
	}

	if (!GetCommTimeouts(iob->fd, &iob->savetimeouts)) {
		eph_error(iob,ERRNO,"GetCommTimeouts error %s",strerror(ERRNO));
		CLOSE(iob->fd);
		return -1;
	}
	iob->worktimeouts.ReadIntervalTimeout=0;
	iob->worktimeouts.ReadTotalTimeoutMultiplier=1;
	iob->worktimeouts.ReadTotalTimeoutConstant=1000;
	iob->worktimeouts.WriteTotalTimeoutMultiplier=0;
	iob->worktimeouts.WriteTotalTimeoutConstant=0;
	if (!SetCommTimeouts(iob->fd, &iob->worktimeouts)) {
/*eph_error(iob,ERRNO,"SetCommTimeouts initial attr error %s",strerror(ERRNO));*/
		CLOSE(iob->fd);
		return -1;
	}

#elif defined(UNIX)

	if (tcgetattr(iob->fd,&tios) < 0) {
/*eph_error(iob,ERRNO,"tcgetattr error %s",strerror(ERRNO));*/
		close(iob->fd);
		return -1;
	}
	memcpy(&iob->savetios,&tios,sizeof(tios));
#ifdef HAVE_CFMAKERAW
	cfmakeraw(&tios);
#else
	tios.c_iflag=0;
	tios.c_oflag=0;
	tios.c_lflag=0;
#endif
	cfsetospeed(&tios,DEFSPEED);
	cfsetispeed(&tios,DEFSPEED);
#ifdef USE_VMIN_AND_VTIME
	tios.c_cc[VMIN]=127;
	tios.c_cc[VTIME]=1;
#else
	tios.c_cc[VMIN]=1;
	tios.c_cc[VTIME]=0;
#endif
	tios.c_cflag|=(CS8|CLOCAL|CREAD);
	tios.c_cflag&=~(PARENB|PARODD
#ifdef TRY_FLOW_CONTROL
# ifdef CRTSXOFF
					|CRTSXOFF
# endif
# ifdef CRTSCTS
						|CRTSCTS
# endif
# ifdef CRTSFL
							|CRTSFL
# endif
#endif
								);
	tios.c_iflag&=~INPCK;
	tios.c_iflag|=IGNBRK;
	if (tcsetattr(iob->fd,TCSANOW,&tios)) {
/*eph_error(iob,ERRNO,"tcsetattr initial attr error %s", strerror(ERRNO));*/
		close(iob->fd);
		return -1;
	}

#endif /* MSWINDOWS/UNIX; do nothing for DOS */

	do {
		eph_writeinit(iob);
		rc=eph_waitsig(iob);
		if (rc) usleep(3000000L);
	} while (rc && (count++ < RETRIES));
	if (rc) {
		CLOSE(iob->fd);
		return -1;
	}

	if (eph_flushinput(iob)) {
/*eph_error(iob,ERRNO,"error flushing input: %s",strerror(ERRNO));*/
		CLOSE(iob->fd);
		return -1;
	}

	if (eph_setispeed(iob,ephspeed)) {
/*eph_error(iob,ERRNO,"could not switch camera speed %d: %s",ephspeed,strerror(ERRNO));*/
		CLOSE(iob->fd);
		return -1;
	}

#if defined(MSWINDOWS)
	dcb.BaudRate = speed;
	if (!SetCommState(iob->fd, &dcb)) {
/*eph_error(iob,ERRNO,"SetCommState working attr error %s", strerror(ERRNO));*/
#elif defined(UNIX)
	cfsetospeed(&tios,tspeed);
	cfsetispeed(&tios,tspeed);
	if (tcsetattr(iob->fd,TCSANOW,&tios)) {
/*eph_error(iob,ERRNO,"tcsetattr working attr error %s",strerror(ERRNO));*/
#elif defined(DOS)
	ttclose();
	TTinit(port, speed);
	if (ttopen()) {
		errno = 0;
/*eph_error(iob,ERRNO,"dobaud set working attr error %s",strerror(ERRNO));*/
#else
#error platform not defined
#endif
		CLOSE(iob->fd);
		return -1;
	}

	usleep(SPEEDCHGDELAY);
	return 0;
}

int eph_close(eph_iob *iob,int switchoff) {

	if (switchoff) {
		char zero=0;

		eph_action(iob,4,&zero,1);
		/* Oly 600 does not send EOT if switched off by command
		eph_waiteot(iob); */
	} else {
		eph_setispeed(iob,0L);
	}

#if defined(MSWINDOWS)
/*
	if (!SetCommState(iob->fd, &iob->savedcb))
eph_error(iob,ERRNO,"SetCommState reset attr error %s",strerror(ERRNO));
	if (!SetCommTimeouts(iob->fd, &iob->savetimeouts))
		eph_error(iob,ERRNO,"SetCommTimeouts reset attr error %s",
					strerror(ERRNO));
*/
#elif defined(UNIX)
	if (tcsetattr(iob->fd,TCSANOW,&iob->savetios)) {
/*eph_error(iob,ERRNO,"tcsetattr cound not reset attr: %s",strerror(ERRNO));*/
	}
#endif /* MSWINDOWS/UNIX; do nothing for DOS */

	return CLOSE(iob->fd);
}
