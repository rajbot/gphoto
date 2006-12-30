/* qcam-FreeBSD.c -- FreeBSD-specific routines for accessing QuickCam */

#include <stdio.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>

#include <machine/cpufunc.h> /* inb & outb */

#include "qcam.h"

static int dev_io = -1;

int read_lpstatus(struct qcam *q) { return inb(q->port+1); }
int read_lpstatus_data(struct qcam *q) { return inw(q->port); }
int read_lpcontrol(struct qcam *q) { return inb(q->port+2); }
int read_lpdata(struct qcam *q) { return inb(q->port); }
void write_lpdata(struct qcam *q, int d) { outb(q->port,d); }
void write_lpcontrol(struct qcam *q, int d) { outb(q->port+2,d); }

int enable_ports(struct qcam *q) 
{
    /* Better safe than sorry */
    if ( q->port < 0x278 || q->port > 0x3bc )
	return 1; 
    if ( dev_io >= 0 )
	return 0;		/* Already got it */
    /* Simply opening /dev/io allows I/O instructions */
    dev_io = open("/dev/io", O_RDWR, 0666);
    return ( dev_io < 0 );
}

int disable_ports(struct qcam *q)
{
    if ( dev_io < 0 )
	return 0;
    close(dev_io);
    dev_io = -1;
    return 0;
}

/* Lock port. */
int qc_lock_wait(struct qcam *q, int wait)
{
    static char lockfile[128];
    static char pid[16];
    int len;

    if ( q->fd < 0 ) {
	sprintf(lockfile, "/tmp/LOCK.qcam.0x%x", q->port);
	q->fd = open(lockfile, O_WRONLY | O_CREAT, 0666);
	if ( q->fd == -1 ) {
	    perror("open");
	    return 1;
	}
#ifdef DEBUG
	fprintf(stderr, "%s - %d: %s open(2)ed\n",
		__FILE__, __LINE__, lockfile);
#endif
    }
    if ( wait ) {
	if ( flock(q->fd, LOCK_EX) == -1 ) {
	    perror("flock");
	    return 1;
	}
    } else {
	if ( flock(q->fd, LOCK_EX | LOCK_NB) == -1 ) {
	    if ( errno != EWOULDBLOCK ) {
		perror("flock");
	    }
	    return 1;
	}
    }
    sprintf(pid, "%ld\n", (long)getpid());
    len = strlen(pid);
    write(q->fd, pid, len);
    ftruncate(q->fd, len);
#ifdef DEBUG
    fprintf(stderr, "%s - %d: fd %d locked exclusively\n",
	    __FILE__, __LINE__, q->fd);
#endif
    return 0;
}

int qc_lock(struct qcam *q)
{
    return qc_lock_wait(q, 1);
}

/* Unlock port */
int qc_unlock(struct qcam *q)
{
    if ( q->fd < 0 ) {
	return 1;
    }
    if ( flock(q->fd, LOCK_UN) == -1 ) {
	perror("flock");
        return 1;
    }
    close(q->fd);
#ifdef DEBUG
    fprintf(stderr, "%s - %d: fd %d unlocked\n", __FILE__, __LINE__, q->fd);
#endif
    q->fd = -1;
  return 0;
}

/* Probe for camera.  Returns 0 if found, 1 if not found, sets q->port. */
int qc_probe(struct qcam *q)
{
    int ioports[] = { 0x378, 0x278, 0x3bc, 0 };
    int i = 0;

    /* Attempt to get permission to access IO ports.  Must be root */
    while( ioports[i] != 0 ) {
	q->port = ioports[i++];
	if ( enable_ports (q) == -1 ) {
	    perror("Can't get I/O permission");
	    exit(1);
	}
	if ( !qc_detect(q) ) {
	    fprintf(stderr, "QuickCam detected at 0x%x\n", q->port);
	    return (0);
	} else
	    qc_close(q);
    }
    qc_close(q);
    return 1;
}
