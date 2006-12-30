#ifndef LINT
static char *rcsid="$Id$";
#endif

/*
	Copyright (c) 1997-1999 Eugene G. Crosser
	Copyright (c) 1998,1999 Bruce D. Lightner (DOS/Windows support)

	You may distribute and/or use for any purpose modified or unmodified
	copies of this software if you preserve the copyright notice above.

	THIS SOFTWARE IS PROVIDED AS IS AND COME WITH NO WARRANTY OF ANY
	KIND, EITHER EXPRESSED OR IMPLIED.  IN NO EVENT WILL THE
	COPYRIGHT HOLDER BE LIABLE FOR ANY DAMAGES RESULTING FROM THE
	USE OF THIS SOFTWARE.
*/

/*
	$Log$
	Revision 1.2  2000/08/24 05:04:27  scottf
	adding language support

	Revision 1.1.1.1.2.1  2000/07/05 11:07:49  ole
	Preliminary support for the Olympus C3030-Zoom USB by
	Fabrice Bellet <Fabrice.Bellet@creatis.insa-lyon.fr>.
	(http://lists.styx.net/archives/public/gphoto-devel/2000-July/003858.html)
	
	Revision 2.8  1999/08/01 21:36:54  crosser
	Modify source to suit ansi2knr
	(I hate the style that ansi2knr requires but you don't expect me
	to write another smarter ansi2knr implementation, right?)

	Revision 2.7  1999/04/22 04:14:54  crosser
	avoid GCC-isms

	Revision 2.6  1999/04/10 16:33:05  lightner
	Used calibrated spin loop for Win32 (like MSDOS) in place of Sleep()
	Speed upspin loop calibrarion for MSDOS

	Revision 2.5  1999/03/12 10:06:23  crosser
	fix typo

	Revision 2.4  1999/03/06 13:37:08  crosser
	Convert to autoconf-style

	Revision 2.3  1998/10/18 13:18:27  crosser
	Put RCS logs and I.D. into the source

	Revision 2.2  1998/02/05 23:34:27  lightner
	Improve DOS logic short usleep() accuracy...use BIOS tick count
	
	Revision 2.1  1998/01/18 02:16:45  crosser
	DOS support
	
	Revision 2.0  1998/01/02 19:20:11  crosser
	Added support for Win32
	
*/

#include <stdio.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#if defined(UNIX)
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_NANOSLEEP
void
usleep(unsigned int useconds)
{
	struct timespec ts;

	ts.tv_sec=(long int)(useconds/1000000);
	ts.tv_nsec=(long int)(useconds%1000000)*1000ul;

	nanosleep (&ts, NULL);
}
#elif HAVE_SELECT
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
void
usleep (unsigned long usec)
{
        struct timeval timeout;

        timeout.tv_sec = usec / 1000000;
        timeout.tv_usec = usec - 1000000 * timeout.tv_sec;
        select(1, NULL, NULL, NULL, &timeout);
}
#else
 # error "cannot sleep: neither nanosleep nor select"
#endif /* HAVE_NANOSLEEP / HAVE_SELECT */

#elif defined(MSWINDOWS)

#include <windows.h>
#include <stdio.h>
#include <sys/timeb.h>
#include <time.h>

static unsigned long start_secs;
static unsigned long start_usecs;
static unsigned long end_usecs;
static int calibrated = 0;
volatile unsigned long dummy = 0;
static double delay_factor = 1;
#define DELAY_MARGIN	1.5

void
start_time(void)
{
	struct _timeb t;

	_ftime(&t);
	start_secs = t.time;
	start_usecs = 1000L * t.millitm;
}

unsigned
long elasped_usecs(void)
{
	struct _timeb t;

	_ftime(&t);
	end_usecs = 1000L * t.millitm + 1000000L * (t.time - start_secs);
	while (end_usecs < start_usecs)
		end_usecs += 60L * 1000000L;
	return end_usecs - start_usecs;
}

unsigned
long spin_loop(double delay_factor)
{
	unsigned long delay = delay_factor;
	unsigned long stop_usecs;

#define CAL_TICKS 2
	dummy = 0;
	start_time();
	if (delay == 0) {
		/* sync with next clock tick */
		while (elasped_usecs() == 0)
			;
		stop_usecs = CAL_TICKS * 55000L + 1000L;
	} else {
		elasped_usecs();
		stop_usecs = -1;
	}
	while (--delay > 0) {
		++dummy;
		if (elasped_usecs() >= stop_usecs)
			break;
	}
	return dummy / CAL_TICKS;
}

void
calibrate_delay(void)
{
#	define MAX_LOOPS 3
	int i;
	unsigned long loops, trial_loops;

	/* calibrate spin_loop() (returns loops per 18.2Hz PC timer tick */
	loops = 0;
        for (i = 0; i < MAX_LOOPS; ++i) {
		trial_loops = spin_loop(0.0);
		if (trial_loops > loops) loops = trial_loops;
	}
	delay_factor = ((double)loops / 55000.0) * DELAY_MARGIN;
	calibrated = 1;
	/* printf("loops = %ld, delay_factor = %g\n", loops, delay_factor); */
}

long
usleep(long usecs)
{
	if (!calibrated)
		calibrate_delay();
	if (usecs > 1000000L) {
		Sleep((DWORD)usecs / 1000L);

	} else {
		spin_loop(delay_factor * usecs + 1);
	}
}

#elif defined(DOS)

#include <dos.h>

static unsigned long start_usecs;
static unsigned long end_usecs;
static int calibrated = 0;
volatile unsigned long dummy = 0;
double delay_factor = 1;
#define DELAY_MARGIN	1.5

#ifndef USE_DOS_TIME
unsigned long stop_tick;

#ifndef NO_TICKS_MACRO
#define get_bios_ticks() (*((volatile unsigned long far *)(MK_FP(0, 0x46c))))
#else /* !NO_TICKS_MACRO */
unsigned long
get_bios_ticks(void)
{
	static unsigned long far *p;

	if (!p)
		p = MK_FP(0, 0x46c);
	return *p;
}
#endif /* !NO_TICKS_MACRO */
#endif /* !USE_DOS_TIME */

void
start_time(void)
{
#ifndef USE_DOS_TIME
	unsigned long t;

	t = get_bios_ticks() & 0xff;
	start_usecs = 55000L * t;
#else
	struct dostime_t t;

	_dos_gettime(&t);
	start_usecs = 1000000L * t.second + 10000L * t.hsecond;
#endif
}

unsigned
long elasped_usecs(void)
{
#ifndef USE_DOS_TIME
	unsigned long t;

	t = get_bios_ticks() & 0xff;
	end_usecs = 55000L * t;
	while (end_usecs < start_usecs)
		end_usecs += 55000L * 0xff;
#else
	struct dostime_t t;

	_dos_gettime(&t);
	end_usecs = 1000000L * t.second + 10000L * t.hsecond;
	while (end_usecs < start_usecs)
		end_usecs += 60L * 1000000L;
#endif
	return end_usecs - start_usecs;
}

unsigned long
spin_loop(double delay_factor)
{
	unsigned long delay = delay_factor;

#ifndef USE_DOS_TIME
#define CAL_TICKS 2
	dummy = 0;
	if (delay == 0) {
		unsigned long end_tick;
		unsigned long start_tick = get_bios_ticks();

		/* sync with next clock tick */
		while (start_tick == (end_tick = get_bios_ticks()))
			;
		stop_tick = end_tick + CAL_TICKS;
	} else {
		stop_tick = get_bios_ticks() - 1;
	}
#endif /* !USE_DOS_TIME */
	while (--delay > 0) {
		++dummy;
#ifndef USE_DOS_TIME
		if (get_bios_ticks() == stop_tick)
			break;
#endif
	}
#ifndef USE_DOS_TIME
	return dummy / CAL_TICKS;
#else
	return dummy;
#endif
}

#ifndef USE_DOS_TIME
void
calibrate_delay(void)
{
#	define MAX_LOOPS 3
	int i;
	unsigned long loops, trial_loops;

	/* calibrate spin_loop() (returns loops per 18.2Hz PC timer tick */
	loops = 0;
        for (i = 0; i < MAX_LOOPS; ++i) {
		trial_loops = spin_loop(0.0);
		if (trial_loops > loops) loops = trial_loops;
	}
	delay_factor = ((double)loops / 55000.0) * DELAY_MARGIN;
	calibrated = 1;
	/* printf("loops = %ld, delay_factor = %g\n", loops, delay_factor); */
}
#else /* USE_DOS_TIME */
void
calibrate_delay(void)
{
	int i;
	unsigned long usecs;

	delay_factor = 1;
	for (i = 0; i < 32; ++i) {
		start_time();
		spin_loop(delay_factor);
		usecs = elasped_usecs();
		if (usecs >= 2L * 55L * 1000L) {
			delay_factor = delay_factor / usecs;
			break;
		}
		delay_factor = delay_factor * 4.0;
	}
	calibrated = 1;
}
#endif /* USE_DOS_TIME */

long
usleep(long usecs)
{
	if (!calibrated)
		calibrate_delay();
	if (usecs > 200000) {
		start_time();
		while (elasped_usecs() < usecs)
			;
	} else {
		spin_loop(delay_factor * usecs + 1);
	}
}

#else
 # error platform not defined
#endif
