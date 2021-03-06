/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gpio-serial.c - Serial IO functions

   Modifications:
   Copyright (C) 2000 Philippe Marzouk <pmarzouk@bigfoot.com>
   Copyright (C) 2000 Edouard Lafargue <Edouard.Lafargue@bigfoot.com>
   Copyright (C) 1999 Johannes Erdfelt <johannes@erdfelt.com>
   Copyright (C) 1999 Scott Fritzinger <scottf@unr.edu>

   Based on work by:
   Copyright (C) 1999 Beat Christen <spiff@longstreet.ch>
   for the toshiba gPhoto library.

   The GPIO Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GPIO Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GPIO Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#if HAVE_TERMIOS_H
#include <termios.h>
#define CRTSCTS  020000000000
#else
#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#include <sgtty.h>
#endif

#include "../include/gpio-serial.h"
#include "../include/gpio.h"

#ifdef HAVE_TERMIOS_H
static struct termios term_old;
#else
static struct sgttyb term_old;
#endif

/* Serial prototypes
   ------------------------------------------------------------------ */
int             gpio_serial_init(gpio_device *dev);
int             gpio_serial_exit(gpio_device *dev);

int             gpio_serial_open(gpio_device *dev);
int             gpio_serial_close(gpio_device *dev);

int             gpio_serial_read(gpio_device *dev, char *bytes, int size);
int             gpio_serial_write(gpio_device *dev, char *bytes, int size);

int             gpio_serial_update (gpio_device *dev);

/* Specific */
int             gpio_serial_get_pin(gpio_device *dev, int pin);
int             gpio_serial_set_pin(gpio_device *dev, int pin, int level);
int             gpio_serial_send_break (gpio_device *dev, int duration);


/* private */
int             gpio_serial_set_baudrate(gpio_device *dev);
static speed_t  gpio_serial_baudconv(int rate);

/* Dynamic library functions
   --------------------------------------------------------------------- */

gpio_device_type gpio_library_type () {

        return (GPIO_DEVICE_SERIAL);
}

gpio_operations *gpio_library_operations () {

        gpio_operations *ops;

        ops = (gpio_operations*)malloc(sizeof(gpio_operations));
        memset(ops, 0, sizeof(gpio_operations));

        ops->init   = gpio_serial_init;
        ops->exit   = gpio_serial_exit;
        ops->open   = gpio_serial_open;
        ops->close  = gpio_serial_close;
        ops->read   = gpio_serial_read;
        ops->write  = gpio_serial_write;
        ops->update = gpio_serial_update;
        ops->get_pin = gpio_serial_get_pin;
        ops->set_pin = gpio_serial_set_pin;
        ops->send_break = gpio_serial_send_break;

        return (ops);
}

int gpio_library_list (gpio_device_info *list, int *count) {


        char buf[1024], prefix[1024];
        int x, fd;
#ifdef __linux
        /* devfs */
        struct stat s;
#endif
#ifdef OS2
        int rc,fh,option;
#endif

        /* Copy in the serial port prefix */
        strcpy(prefix, GPIO_SERIAL_PREFIX);

#ifdef __linux
        /* devfs */
        if (stat("/dev/tts", &s)==0)
                strcpy(prefix, "/dev/tts/%i");
#endif
        for (x=GPIO_SERIAL_RANGE_LOW; x<=GPIO_SERIAL_RANGE_HIGH; x++) {
                sprintf(buf, prefix, x);
#ifdef OS2
           rc = DosOpen(buf,&fh,&option,0,0,1,OPEN_FLAGS_FAIL_ON_ERROR|OPEN_SHARE_DENYREADWRITE,0);
           DosClose(fh);
           if(rc==0) {
#endif
                fd = open (buf, O_RDONLY | O_NDELAY);
                if (fd != -1) {
                        close(fd);
                        list[*count].type = GPIO_DEVICE_SERIAL;
                        strcpy(list[*count].path, buf);
                        sprintf(buf, "Serial Port %i", x);
                        strcpy(list[*count].name, buf);
                        list[*count].argument_needed = 0;
                        *count += 1;
                }
#ifdef OS2
           }
#endif
        }

        return (GPIO_OK);
}

/* Serial API functions
   ------------------------------------------------------------------ */

int gpio_serial_init (gpio_device *dev) {
        /* save previous setttings in to dev->settings_saved */
#if HAVE_TERMIOS_H
        if (tcgetattr(dev->device_fd, &term_old) < 0) {
                perror("tcgetattr");
                return GPIO_ERROR;
        }
#else
        if (ioctl(dev->device_fd, TIOCGETP, &term_old) < 0) {
                perror("ioctl(TIOCGETP)");
                return GPIO_ERROR;
        }
#endif
        return GPIO_OK;
}

int gpio_serial_exit (gpio_device *dev) {
        /* ... */
        return GPIO_OK;
}

int gpio_serial_open(gpio_device * dev)
{

#ifdef __FreeBSD__
        dev->device_fd = open(dev->settings.serial.port, O_RDWR | O_NOCTTY | O_NONBLOCK);
#else
        dev->device_fd = open(dev->settings.serial.port, O_RDWR | O_NOCTTY | O_SYNC | O_NONBLOCK);
#endif
        if (dev->device_fd == -1) {
                fprintf(stderr, "gpio_serial_open: failed to open ");
                perror(dev->settings.serial.port);
                return GPIO_ERROR;
        }

/*      if (ioctl (dev->device_fd, TIOCMBIC, &RTS) <0) {
                perror("ioctl(TIOCMBIC)");
                return GPIO_ERROR;
        } */
        return GPIO_OK;
}

int gpio_serial_close(gpio_device * dev)
{
        if (close(dev->device_fd) == -1) {
                perror("gpio_serial_close: tried closing device file descriptor");
                return GPIO_ERROR;
        }
        return GPIO_OK;
}

int gpio_serial_write(gpio_device * dev, char *bytes, int size)
{
        int len, ret;

        len = 0;
        while (len < size) {    /* Make sure we write all data while handling */
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
#ifndef OS2
#if HAVE_TERMIOS_H
        tcdrain(dev->device_fd);
#else
        ioctl(dev->device_fd, TCDRAIN, 0);
#endif
#endif
        return GPIO_OK;
}


int gpio_serial_read(gpio_device * dev, char *bytes, int size)
{
        struct timeval timeout;
        fd_set readfs;          /* file descriptor set */
        int readen = 0;
        int rc;

        FD_ZERO(&readfs);
        FD_SET(dev->device_fd, &readfs);

        while (readen < size) {
                /* set timeout value within input loop */
                timeout.tv_usec = (dev->timeout % 1000) * 1000;
                timeout.tv_sec = (dev->timeout / 1000);  /* = 0
                                                          * if dev->timeout < 1000
                                                          */


                rc = select(dev->device_fd + 1, &readfs, NULL, NULL, &timeout);
/*              if ( (rc == 0) && (readen == 0)) { */
                /* Timeout before reading anything */
/*                printf("gpio_serial_read (timeout)\n"); */
/*                return GPIO_TIMEOUT; */
/*              } */
                if (0 == rc) {
                        return GPIO_TIMEOUT;
                }
                if (FD_ISSET(dev->device_fd, &readfs)) {
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
                        return GPIO_ERROR;
                }
        }
        return readen;
}

/*
 * Get the status of the lines of the serial port
 *
 */
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

/*
* Set the status of lines in the serial port
*
* level is 0 for off and 1 for on
*
*/
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

/*
 * This function will apply the settings to
 * the device. The device has to be opened
 */
int gpio_serial_update(gpio_device * dev)
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
int gpio_serial_set_baudrate(gpio_device * dev)
{
#if HAVE_TERMIOS_H
        struct termios tio;

        if (tcgetattr(dev->device_fd, &tio) < 0) {
                perror("tcgetattr");
                return GPIO_ERROR;
        }
        tio.c_cflag = (tio.c_cflag & ~CSIZE) | CS8;

        /* Set into raw, no echo mode */
#if defined(__FreeBSD__) || defined(__NetBSD__)
        tio.c_iflag &= ~(IGNBRK | IGNCR | INLCR | ICRNL |
                         IXANY | IXON | IXOFF | INPCK | ISTRIP);
#else
        tio.c_iflag &= ~(IGNBRK | IGNCR | INLCR | ICRNL | IUCLC |
                         IXANY | IXON | IXOFF | INPCK | ISTRIP);
#endif
        tio.c_iflag |= (BRKINT | IGNPAR);
        tio.c_oflag &= ~OPOST;
        tio.c_lflag &= ~(ICANON | ISIG | ECHO | ECHONL | ECHOE |
                         ECHOK | IEXTEN);
        tio.c_cflag &= ~(CRTSCTS | PARENB | PARODD);
        tio.c_cflag |= CLOCAL | CREAD;

        tio.c_cc[VMIN] = 1;
        tio.c_cc[VTIME] = 0;

        cfsetispeed(&tio, gpio_serial_baudconv(dev->settings.serial.speed));
        cfsetospeed(&tio, gpio_serial_baudconv(dev->settings.serial.speed));

        if (tcsetattr(dev->device_fd, TCSANOW, &tio) < 0) {
                perror("tcsetattr");
                return GPIO_ERROR;
        }
        if (fcntl(dev->device_fd, F_SETFL, 0) < 0) {    /* clear O_NONBLOCK */
                perror("fcntl F_SETFL");
                return -1;
        }
#else
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
#endif

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

int gpio_serial_send_break (gpio_device *dev, int duration) {

        /* Duration is in seconds */

#if HAVE_TERMIOS_H
        tcsendbreak(dev->device_fd, duration / 3);
        tcdrain(dev->device_fd);
#else
        /* ioctl */
#endif
        return 0;
}
