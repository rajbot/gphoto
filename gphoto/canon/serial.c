/****************************************************************************
 *
 * File: serial.c 
 *
 * Serial communication layer.
 *
 ****************************************************************************/

/****************************************************************************
 *
 * include files
 *
 ****************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/time.h>
#include <errno.h>

#include "util.h"

#define D(c) c

/****************************************************************************
 *
 * static global storage area
 *
 ****************************************************************************/

static struct termios oldtio, newtio;
static int fd;

/*****************************************************************************
 *
 * canon_serial_init
 *
 * Initializes the given serial device.
 *
 * devname - the name of the device to open 
 *
 * Returns 0 on success.
 * Returns -1 on any error.
 *
 ****************************************************************************/

int canon_serial_init(const char *devname)
{
    if (!devname)
    {
	fprintf(stderr, "INVALID device string (NULL)\n");
	return -1;
    }

    D(printf("canon_init_serial(): devname %s\n", devname));

    fd = open(devname, O_RDWR | O_NOCTTY | O_SYNC | O_NONBLOCK);
    if (fd < 0)
    {
	perror(devname);
	return -1;
    }

    if (0 > tcgetattr(fd, &oldtio))
    {
	perror("canon_init_serial(): tcgetattr()");
	close(fd);
	return -1;
    }

    newtio = oldtio;
    newtio.c_cflag = (newtio.c_cflag & ~CSIZE) | CS8;

    /* Set into raw, no echo mode */
    newtio.c_iflag &= ~(IGNBRK | IGNCR | INLCR | ICRNL | IUCLC |
			IXANY | IXON | IXOFF | INPCK | ISTRIP);
    newtio.c_iflag |= (BRKINT | IGNPAR);
    newtio.c_oflag &= ~OPOST;
    newtio.c_lflag = ~(ICANON | ISIG | ECHO | ECHONL | ECHOE | ECHOK);
    newtio.c_cflag &= ~(CRTSCTS | PARENB | PARODD);
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cc[VMIN] = 1;
    newtio.c_cc[VTIME] = 0;

    /* set speed */
    cfsetospeed(&newtio, B9600);
    cfsetispeed(&newtio, B9600);

    if (0 > tcsetattr(fd, TCSANOW, &newtio))
    {
	perror("canon_init_serial(): tcsetattr()");
	close(fd);
	return -1;
    }

    if (fcntl(fd,F_SETFL,0) < 0) { /* clear O_NONBLOCK */
	perror("fcntl F_SETFL");
	return -1;
    }

    return 0;
}

/*****************************************************************************
 *
 * canon_serial_restore
 *
 * Restores the saved settings for the serial device
 * 
 * Returns 0 on success.
 * Returns -1 on any error.
 *
 ****************************************************************************/

int canon_serial_restore()
{
    int rc = 0;

    if (0 > tcsetattr(fd, TCSANOW, &oldtio))
    {
	perror("canon_restore_serial(): tcsetattr()");
    }
    close(fd);

    return rc;
}

/*****************************************************************************
 *
 * canon_serial_send
 *
 * Send the given buffer with given length over the serial line.
 *
 * buf   - the raw data buffer to send
 * len   - the length of the buffer
 *
 * Returns 0 on success, -1 on error.
 *
 ****************************************************************************/

int canon_serial_send(const unsigned char *buf, int len)
{
    D(dump_hex("canon_serial_send()", buf, len));

    while (len > 0)
    {
	int sent = write(fd, buf, len);

	if (sent < 0)
	{
	    if (EINTR == errno)
	    {
		continue;
	    }
perror("serial");
	    return -1;
	}
	len -= sent;
	buf += sent;
    }
    return 0;
}

/*****************************************************************************
 *
 * canon_serial_get_byte
 *
 * Get the next byte from the serial line.
 * Actually the fucntion reads chunks of data and keeps them in a cache.
 * Only one byte per call will be returned.
 *
 * Returns the byte on success, -1 on error.
 *
 ****************************************************************************/

int to_secs = 1;
void serial_set_timeout(int to)
{
  to_secs = to;
}

int canon_serial_get_byte()
{
    static unsigned char cache[512];
    static unsigned char *cachep = cache;
    static unsigned char *cachee = cache;

    int rc;
    fd_set readfs;
    struct timeval timeout;

    /* if still data in cache, get it */
    if (cachep < cachee)
    {
	return (int) *cachep++;
    }

    /* otherwise read from device */
    FD_ZERO(&readfs);
    FD_SET(fd, &readfs);

    /* set the timout; initial detection works with 0.9-1.04 sec */
    timeout.tv_sec = to_secs;
    timeout.tv_usec = 0;

    rc = select(fd + 1, &readfs, NULL, NULL, &timeout);

    if (0 == rc)
    {
	D(fprintf(stderr, "###### canon_serial_get_byte(): recv timeout #############################\n"));
	return -1;
    }
    if (rc < 0)
    {
	D(fprintf(stderr, "canon_serial_get_byte(): recv error\n"));
	return -1;
    }

    if (FD_ISSET(fd, &readfs))
    {
	int recv = read(fd, cache, sizeof(cache));

	cachep = cache;
	cachee = cache + recv;

	if (recv)
	{
	    return (int) *cachep++;
	}
    }
    return -1;
}

/****************************************************************************
 *
 * End of file: serial.c
 *
 ****************************************************************************/
