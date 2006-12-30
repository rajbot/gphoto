/*
	The (gPhoto) I/O Library (serial)

	Modified by:
	Copyright 1999, Scott Fritzinger <scottf@unr.edu>
	Copyright 1999, Johannes Erdfelt <johannes@erdfelt.com>

	Based on serial port code by:
	Copyright 1999, Beat Christen <spiff@longstreet.ch>
		for the toshiba gPhoto library.

	This library is covered by the LGPL.
*/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#if HAVE_TERMIOS_H 
# include <termios.h>
#else                   
#  if HAVE_SYS_IOCTL_H
#   include <sys/ioctl.h>
#  endif
#  include <sgtty.h>
# endif

#include "gpio.h"
#include "serial.h"

#ifdef HAVE_TERMIOS_H 
	static struct termios term_old;
#else
	static struct sgttyb term_old;
#endif

/* Private Serial functions
   ------------------------------------------------------------- */
int gpio_serial_init(gpio_device *dev)
{
	/* save previous setttings in to dev->settings_saved */
#if HAVE_TERMIOS_H 
	if (tcgetattr(dev->device_fd,&term_old) < 0) {
		perror("tcgetattr");
		return GPIO_ERROR;
	}
#else
	if (ioctl(dev->device_fd, TIOCGETP,&term_old) < 0) {
		perror("ioctl(TIOCGETP)");
		return GPIO_ERROR;
	}
#endif
	return GPIO_OK;
}


int gpio_serial_open(gpio_device *dev)
{
	dev->device_fd = open(dev->settings.serial.port, O_RDWR | O_NDELAY);
	if (dev->device_fd == -1) {
		fprintf(stderr, "gpio_serial_open: failed to open ");
		perror(dev->settings.serial.port);
		return GPIO_ERROR;
	}

	return GPIO_OK;
}

int gpio_serial_close(gpio_device *dev)
{
	if (close(dev->device_fd) == -1) {
		perror("gpio_serial_close: tried closing device file descriptor");
		return GPIO_ERROR;
	}

	return GPIO_OK;
}

int gpio_serial_write(gpio_device *dev, char *bytes, int size)
{
	int len, ret;

	len = 0;
	while (len < size) {	/* Make sure we write all data while handling */
				/* the harmless errors */
		if ((ret = write(dev->device_fd, bytes, size - len)) == -1)
			switch (errno) {
			case EAGAIN:
			case EINTR:
				ret = 0;  
				break;
			default:
				perror("gpio_serial_write");
				return GPIO_ERROR;
			}

		len += ret;
	}

	/* wait till all bytes are really sent */
#if HAVE_TERMIOS_H
	tcdrain(dev->device_fd);
#else
	ioctl(dev->device_fd, TCDRAIN, 0);
#endif
	return GPIO_OK;
}


int gpio_serial_read(gpio_device *dev, char *bytes, int size)
{
	struct timeval timeout;
	fd_set 	readfs;    /* file descriptor set */
	int     readen = 0;
	
	FD_ZERO(&readfs);
	FD_SET(dev->device_fd, &readfs);
	
	while (readen < size) {
		/* set timeout value within input loop */
		timeout.tv_usec = dev->timeout * 1000; 
		timeout.tv_sec  = 0;
			
		select(dev->device_fd + 1, &readfs, NULL, NULL, &timeout);
		if (FD_ISSET(dev->device_fd,&readfs)) {
			int now = read(dev->device_fd, bytes, size - readen);
			
			if (now < 0) {
				perror("gpio_serial_read (read fails)");
				return GPIO_ERROR;
			} else {
				bytes += now;
				readen += now;
			}
		} else {
			perror("gpio_serial_read (tty timeout)");
			return GPIO_TIMEOUT;
		}
	}
	return readen;
}


/*
 * This function will apply the settings to
 * the device. The device has to be opened
 */
int gpio_serial_update (gpio_device *dev)
{
	memcpy(&dev->settings, &dev->settings_pending, sizeof(dev->settings));

	if (dev->device_fd != 0) {
		if (gpio_serial_close(dev) == GPIO_ERROR)
			return GPIO_ERROR;
		if (gpio_serial_open(dev) == GPIO_ERROR)
			return GPIO_ERROR;

		return gpio_serial_set_baudrate(dev);
	}

	return GPIO_OK;
}

/*
   Serial port specific helper functions
   ----------------------------------------------------------------
*/

/* Called to set the baud rate */
int gpio_serial_set_baudrate(gpio_device *dev)
{
#if HAVE_TERMIOS_H
	struct termios tio;
  
	if (tcgetattr(dev->device_fd, &tio) < 0) {
		perror("tcgetattr");
		return GPIO_ERROR;
	}

	tio.c_iflag = 0;
	tio.c_oflag = 0;
	tio.c_cflag = CS8 | CREAD | CLOCAL;
	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 5;

	tio.c_lflag &= ~(ICANON | ISIG | ECHO | ECHONL | ECHOE | ECHOK);

	cfsetispeed(&tio, gpio_serial_baudconv(dev->settings.serial.speed));
	cfsetospeed(&tio, gpio_serial_baudconv(dev->settings.serial.speed));

	if (tcsetattr(dev->device_fd, TCSANOW | TCSAFLUSH, &tio) < 0) {
		perror("tcsetattr");
		return GPIO_ERROR;
	}
# else
	struct sgttyb ttyb;
  
	if (ioctl(dev->device_fd, TIOCGETP, &ttyb) < 0) {
		perror("ioctl(TIOCGETP)");
		return GPIO_ERROR;
	}

	ttyb.sg_ispeed = dev->settings.serial.speed;
	ttyb.sg_ospeed = dev->settings.serial.speed;
	ttyb.sg_flags = 0;

	if (ioctl(dev->device_fd, TIOCSETP, &ttyb) < 0) {
		perror("ioctl(TIOCSETP)");
		return GPIO_ERROR;
	}
# endif

	return GPIO_OK;
}

/* Called to convert a int baud to the POSIX enum value */
static speed_t gpio_serial_baudconv(int baud)
{
#define BAUDCASE(x)     case (x): { ret = B##x; break; }
	speed_t ret;
  
	ret = (speed_t) baud;
	switch (baud) {
	/* POSIX defined baudrates */
	BAUDCASE(0);
	BAUDCASE(50);
	BAUDCASE(75);
	BAUDCASE(110);
	BAUDCASE(134);
	BAUDCASE(150);
	BAUDCASE(200);
	BAUDCASE(300);
	BAUDCASE(600);
	BAUDCASE(1200);
	BAUDCASE(1800);
	BAUDCASE(2400);
	BAUDCASE(4800);
	BAUDCASE(9600);
	BAUDCASE(19200);
	BAUDCASE(38400);

	/* non POSIX values */
#ifdef B7200
	BAUDCASE(7200);
#endif
#ifdef B14400
	BAUDCASE(14400);
#endif
#ifdef B28800
	BAUDCASE(28800);
#endif
#ifdef B57600
	BAUDCASE(57600);
#endif
#ifdef B115200
	BAUDCASE(115200);
#endif
#ifdef B230400
	BAUDCASE(230400);
#endif
    
	default:
		fprintf(stderr, "baudconv: baudrate %d is undefined; using as is\n", baud);
	}
  
	return ret;
#undef BAUDCASE
}

int gpio_serial_set_pin(gpio_device * dev, int pin, int level)
{
        int bit,request;

        switch(pin) {   
                case PIN_RTS:
                        bit = TIOCM_RTS;
                        break;
                case PIN_DTR:
                        bit = TIOCM_DTR;
                        break;
                case PIN_CTS:
                        bit = TIOCM_CTS;
                        break;
                case PIN_DSR:
                        bit = TIOCM_DSR;
                        break;
                case PIN_CD:
                        bit = TIOCM_CD;
                        break;
                case PIN_RING:
                        bit = TIOCM_RNG;
                        break;
                default:
                        return GPIO_ERROR;
        }

        switch(level) {
                case 0:
                        request = TIOCMBIS;
                        break;
                case 1:
                        request = TIOCMBIC;
                        break;
                default:
                        return GPIO_ERROR;
        }

        if (ioctl (dev->device_fd, request, &bit) <0) {
        perror("ioctl(TIOCMBI[CS])");
        return GPIO_ERROR;
    }

        return GPIO_OK;
}

int gpio_serial_get_pin(gpio_device * dev, int pin)
{
        int j, bit;

        switch(pin) {
                case PIN_RTS:
                        bit = TIOCM_RTS;
                        break;
                case PIN_DTR:
                        bit = TIOCM_DTR;
                        break;
                case PIN_CTS:
                        bit = TIOCM_CTS;
                        break;
                case PIN_DSR:
                        bit = TIOCM_DSR;
                        break;   
                case PIN_CD:
                        bit = TIOCM_CD;
                        break;
                case PIN_RING:
                        bit = TIOCM_RNG;
                        break;
                default:
                        return GPIO_ERROR;
        }

        if (ioctl(dev->device_fd, TIOCMGET, &j) < 0) {
                perror("gpio_serial_status (Getting hardware status bits)");
                return GPIO_ERROR;
        }
        return (j & bit);
}


struct gpio_operations gpio_serial_operations = {
	gpio_serial_init, 
	gpio_serial_open,
	gpio_serial_close,
	gpio_serial_read,
	gpio_serial_write,
	gpio_serial_update,
	gpio_serial_get_pin,
	gpio_serial_set_pin
};
