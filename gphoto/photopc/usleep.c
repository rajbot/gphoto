
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

	Revision 2.2  1998/02/05 23:34:27  lightner
	Improve DOS logic short usleep() accuracy...use BIOS tick count
	
	Revision 2.1  1998/01/18 02:16:45  crosser
	DOS support
	
	Revision 2.0  1998/01/02 19:20:11  crosser
	Added support for Win32
	
*/

#if defined(UNIX)
#include <time.h>
#include <unistd.h>

#ifdef HAVE_NANOSLEEP
void usleep (unsigned int useconds) {
  struct timespec ts = { tv_sec: (long int) (useconds / 1000000),
			 tv_nsec: (long int) (useconds % 1000000) * 1000ul };

  __nanosleep (&ts, NULL);
}
#else /* HAVE_NANOSLEEP */
#ifdef HAVE_SELECT_H
#include <sys/select.h>
#endif
void usleep(unsigned long usec) {
        struct timeval timeout;

        timeout.tv_sec = usec / 1000000;
        timeout.tv_usec = usec - 1000000 * timeout.tv_sec;
        select(1, NULL, NULL, NULL, &timeout);
}
#endif /* HAVE_NANOSLEEP */

#elif defined(MSWINDOWS)

#include <windows.h>

void usleep(long usecs) {
	long msecs;

	msecs = usecs / 1000L;
	if (msecs <= 0) msecs = 1;	/* minimum delay is 1 millisecond */
	Sleep((DWORD)msecs);
}

#elif defined(DOS)

#include <dos.h>

static unsigned long start_usecs;
static unsigned long end_usecs;
static int calibrated = 0;
volatile unsigned long dummy = 0;
double delay_factor = 1;

#ifndef USE_DOS_TIME
unsigned long stop_tick;

#ifndef NO_TICKS_MACRO
#define get_bios_ticks() (*((volatile unsigned long far *)(MK_FP(0, 0x46c))))
#else /* !NO_TICKS_MACRO */
unsigned long get_bios_ticks(void)
{
	static unsigned long far *p;

	if (!p)
		p = MK_FP(0, 0x46c);
	return *p;
}
#endif /* !NO_TICKS_MACRO */
#endif /* !USE_DOS_TIME */

void start_time(void) {
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

unsigned long elasped_usecs(void) {
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

unsigned long spin_loop(double delay_factor)
{
	unsigned long delay = delay_factor;

#ifndef USE_DOS_TIME
#define CAL_TICKS 4
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
};

#ifndef USE_DOS_TIME
void calibrate_delay(void)
{
	unsigned long loops, loops1, loops2;

	/* calibrate spin_loop() (returns loops per 18.2Hz PC timer tick */
	loops1 = spin_loop(0.0);
	loops2 = spin_loop(0.0);
	/* use larger loop count for delay factor calculation*/
	loops = (loops1 > loops2) ? loops1 : loops2;
	delay_factor = (double)loops / 55000.0;
	calibrated = 1;
}
#else /* USE_DOS_TIME */
void calibrate_delay(void)
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

long usleep(long usecs)
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
#error platform not defined
#endif
