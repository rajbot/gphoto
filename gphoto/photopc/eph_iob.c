
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
	
	Revision 2.3  1998/10/18 13:18:27  crosser
	Put RCS logs and I.D. into the source

	Revision 2.2  1998/02/08 19:58:38  crosser
	Support low memory: chunked saving etc.
	
	Revision 2.1  1998/01/18 02:16:45  crosser
	DOS support
	
	Revision 2.0  1998/01/02 19:20:11  crosser
	Added support for Win32
	
	Revision 1.1  1997/08/17 08:59:54  crosser
	Initial revision
	
*/

#include "eph_io.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void deferrorcb(int errcode,char *errstr) {
	return;
	// fprintf(stderr,"Error %d: %s\n",errcode,errstr);
}

static void *defrealloccb(void *old,size_t length) {
	if (old) return realloc(old,length);
	else return malloc(length);
}

static void defruncb(off_t count) {
	return;
}

eph_iob *eph_new(void (*errorcb)(int errcode,char *errstr),
		void *(*realloccb)(void *old,size_t length),
		void (*runcb)(off_t count),
		int (*storecb)(char *data,size_t size),
		int debug) {
	eph_iob *iob;

	iob=(eph_iob *)malloc(sizeof(eph_iob));
	if (!iob) return iob;
	if (errorcb) iob->errorcb=errorcb;
	else iob->errorcb=deferrorcb;
	if (realloccb) iob->realloccb=realloccb;
	else iob->realloccb=defrealloccb;
	if (runcb) iob->runcb=runcb;
	else iob->runcb=defruncb;
	if (storecb) iob->storecb=storecb;
	else iob->storecb=NULL;
	iob->debug=debug;
#if defined(MSWINDOWS)
	iob->fd=INVALID_HANDLE_VALUE;
	memset((char*)&iob->savedcb,0,sizeof(iob->savedcb));
	memset((char*)&iob->savetimeouts,0,sizeof(iob->savetimeouts));
#elif defined(UNIX)
	iob->fd=-1;
	memset((char*)&iob->savetios,0,sizeof(iob->savetios));
#elif defined(DOS)
	iob->fd=-1;
#else
#error "platform not defined"
#endif
	return iob;
}

void eph_free(eph_iob *iob) {
	free(iob);
}
