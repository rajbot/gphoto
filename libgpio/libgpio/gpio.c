/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gpio.c - Core IO library functions

   Modifications:
   Copyright (C) 1999 Scott Fritzinger <scottf@unr.edu>

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

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../include/gpio.h"
#include "library.h"

gpio_device_info device_list[256];
int              device_count;

/* Toggle to turn on/off debugging */
int              glob_debug_level=0;

void gpio_debug_printf (int target_debug_level, int debug_level, char *format, ...)
{
        va_list arg;

        if ((debug_level > 0)&&(debug_level >= target_debug_level)) {
                fprintf(stderr, "gpio: ");
                va_start(arg, format);
                vfprintf(stderr, format, arg);
                va_end(arg);
                fprintf(stderr, "\n");
        }
}

/*
   Required library functions
   ----------------------------------------------------------------
 */

int gpio_init(int debug)
{
        gpio_debug_printf(GPIO_DEBUG_LOW, glob_debug_level, "Initializing...");
        /* Enumerate all the available devices */
        device_count = 0;
	glob_debug_level = debug;
        return (gpio_library_list(device_list, &device_count));
}

int gpio_get_device_count(void)
{
        gpio_debug_printf(GPIO_DEBUG_LOW, glob_debug_level, "Device count: %i", device_count);
        return device_count;
}

int gpio_get_device_info(int device_number, gpio_device_info *info)
{
        gpio_debug_printf(GPIO_DEBUG_LOW, glob_debug_level, "Getting device info...");

        memcpy(info, &device_list[device_number], sizeof(device_list[device_number]));

        return GPIO_OK;
}

gpio_device *gpio_new(gpio_device_type type)
        /* Create a new IO device */
{
        gpio_device *dev;
        gpio_device_settings settings;
        char buf[1024];

        gpio_debug_printf(GPIO_DEBUG_LOW, glob_debug_level, "Creating new device... ");

        dev = (gpio_device *) malloc(sizeof(gpio_device));
        if (!dev) {
                gpio_debug_printf(GPIO_DEBUG_LOW, glob_debug_level, "Can not allocate device!");
                return NULL;
        }
	memset(dev, 0, sizeof(gpio_device));

        if (gpio_library_load(dev, type)) {
                /* whoops! that type of device isn't supported */
                gpio_debug_printf(GPIO_DEBUG_LOW, glob_debug_level, "Device type not supported! (%i)", type);
                free(dev);
                return NULL;
        }

	dev->debug_level = glob_debug_level;

        dev->type = type;
        dev->device_fd = 0;
	dev->ops->init(dev);

        switch (dev->type) {
        case GPIO_DEVICE_SERIAL:
                sprintf(buf, GPIO_SERIAL_PREFIX, GPIO_SERIAL_RANGE_LOW);
                strcpy(settings.serial.port, buf);
                /* set some defaults */
                settings.serial.speed = 9600;
                settings.serial.bits = 8;
                settings.serial.parity = 0;
                settings.serial.stopbits = 1;
                gpio_set_settings(dev, settings);
                gpio_set_timeout(dev, 500);
                break;
        case GPIO_DEVICE_PARALLEL:
#ifdef GPIO_PARALLEL
                sprintf(buf, GPIO_SERIAL_PREFIX, GPIO_SERIAL_RANGE_LOW);
                strcpy(settings.parallel.port, buf);
#endif
                break;
        case GPIO_DEVICE_NETWORK:
#ifdef GPIO_NETWORK
                gpio_set_timeout(dev, 50000);
#endif
                break;
        case GPIO_DEVICE_USB:
#ifdef GPIO_USB
                gpio_set_timeout(dev, 5000);
#endif
                break;
        case GPIO_DEVICE_IEEE1394:
#ifdef GPIO_IEEE1394
                /* blah ? */
#endif
                break;
        default:
                /* ERROR! */
                break;
        }

        gpio_debug_printf(GPIO_DEBUG_LOW, glob_debug_level, "Created device successfully...");

        return (dev);
}

int gpio_set_debug (gpio_device *dev, int debug_level)
{
	dev->debug_level = debug_level;

	return (GPIO_OK);
}

int gpio_open(gpio_device *dev)
        /* Open a device for reading/writing */
{
        int retval = 0;

        /* Try to open device */
        retval = dev->ops->open(dev);
        if (retval == GPIO_OK) {
                /* Now update the settings */
                retval = dev->ops->update(dev);
                if (retval != GPIO_OK) {
                        dev->device_fd = 0;
			gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level, 
				"gpio_open: update error");
                        return GPIO_ERROR;
                }
		gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level, "gpio_open: OK");
                return GPIO_OK;
        }
	gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level, "gpio_open: open error");
        return GPIO_ERROR;
}

int gpio_close(gpio_device *dev)
        /* Close the device to prevent reading/writing */
{
        int retval = 0;

        if (!dev) {
		gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level, "gpio_close: bad device");
                return GPIO_ERROR;
	}
        if (dev->type == GPIO_DEVICE_SERIAL && dev->device_fd == 0) {
		gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level, "gpio_close: OK");
                return GPIO_OK;
	}

        retval = dev->ops->close(dev);
        dev->device_fd = 0;
	gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level, 
		"gpio_close: close %s", retval == GPIO_OK? "ok":"error");
        return retval;
}

int gpio_free(gpio_device *dev)
        /* Frees a device struct */
{
        int retval = dev->ops->exit(dev);

	gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level,
		"gpio_free: exit %s", retval < 0? "error":"ok");

        gpio_library_close(dev);
        free(dev);

        return GPIO_OK;
}

int gpio_write(gpio_device *dev, char *bytes, int size)
        /* Called to write "bytes" to the IO device */
{
	int x, retval;
	char t[8];
	char *buf;

	if (glob_debug_level == GPIO_DEBUG_HIGH) {
		buf = (char *)malloc(sizeof(char)*(4*size+64));
		buf[0] = 0;
		for (x=0; x<size; x++) {
			sprintf(t, "%02x ", (unsigned char)bytes[x]);
			strcat(buf, t);
		}
		gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level,
			"gpio_write: (size=%05i) DATA: %s", size, buf);
		free(buf);
	}
        retval =  dev->ops->write(dev, bytes, size);

	if (retval == GPIO_TIMEOUT)
		gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level, "gpio_write: write timeout");
	if (retval == GPIO_ERROR)
		gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level, "gpio_write: write error");

	return (retval);
}

int gpio_read(gpio_device *dev, char *bytes, int size)
        /* Reads data from the device into the "bytes" buffer.
           "bytes" should be large enough to hold all the data.
         */
{
	int x, retval;	
	char t[8];
	char *buf;

	retval = dev->ops->read(dev, bytes, size);

	if ((retval > 0)&&(glob_debug_level == GPIO_DEBUG_HIGH)) {
		buf = (char *)malloc(sizeof(char)*(4*retval+64));
		buf[0] = 0;
		for (x=0; x<retval; x++) {
			sprintf(t, "%02x ", (unsigned char)bytes[x]);
			strcat(buf, t);
		}
		gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level, 
			"gpio_read:  (size=%05i) DATA: %s", retval, buf);
		free(buf);
	}

	if (retval == GPIO_TIMEOUT)
		gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level, "gpio_read: read timeout");
	if (retval == GPIO_ERROR)
		gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level, "gpio_read: read error");

	return (retval);
}

int gpio_set_timeout(gpio_device *dev, int millisec_timeout)
{
        dev->timeout = millisec_timeout;

	gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level, 
		"gpio_set_timeout: value=%ims", millisec_timeout);

        return GPIO_OK;
}

int gpio_get_timeout(gpio_device *dev, int *millisec_timeout)
{
        *millisec_timeout = dev->timeout;

	gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level,
		"gpio_get_timeout: value=%ims", *millisec_timeout);

        return GPIO_OK;
}

int gpio_set_settings(gpio_device *dev, gpio_device_settings settings)
{
	int retval;

        /* need to memcpy() settings to dev->settings */
        memcpy(&dev->settings_pending, &settings, sizeof(dev->settings_pending));

        retval =  dev->ops->update(dev);
	gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level,
		"gpio_set_settings: update %s", retval < 0? "error":"ok");
	return (retval);
}


int gpio_get_settings(gpio_device *dev, gpio_device_settings * settings)
{
        memcpy(settings, &dev->settings, sizeof(gpio_device_settings));
	gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level, "gpio_get_settings: ok");

        return GPIO_OK;
}

/* Serial and Parallel-specific functions */
/* ------------------------------------------------------------------ */

int gpio_get_pin(gpio_device *dev, int pin)
{
	int retval;

        if (!dev->ops->get_pin) {
		gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level, "gpio_get_pin: get_pin NULL");
                return (GPIO_ERROR);
	}

        retval = dev->ops->get_pin(dev, pin);
	gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level, 
		"gpio_get_pin: get_pin %s", retval < 0? "error":"ok");
	return (retval);
}

int gpio_set_pin(gpio_device *dev, int pin, int level)
{
	int retval;

        if (!dev->ops->get_pin) {
		gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level, "gpio_set_pin: set_pin NULL");
                return (GPIO_ERROR);
	}

        retval = dev->ops->set_pin(dev, pin, level);
	gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level,
		"gpio_set_pin: set_pin %s", retval < 0? "error":"ok");
	return (retval);
}

int gpio_send_break (gpio_device *dev, int duration)
{
	int retval;

        if (!dev->ops->send_break) {
		gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level, "gpio_break: gpio_break NULL");
                return (GPIO_ERROR);
	}

        retval = dev->ops->send_break(dev, duration);
	gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level,
		"gpio_send_break: send_break %s", retval < 0? "error":"ok");
	return (retval);
}

/* USB-specific functions */
/* ------------------------------------------------------------------ */

#ifdef GPIO_USB

int gpio_usb_find_device (gpio_device * dev, int idvendor, int idproduct)
{
	int retval;

        if (!dev->ops->find_device) {
		gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level,
			"gpio_usb_find_device: find_device NULL");
                return (GPIO_ERROR);
	}

        retval = dev->ops->find_device(dev, idvendor, idproduct);
	gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level, 
		"gpio_usb_find_device: find_device (0x%04x 0x%04x) %s", 
		idvendor, idproduct, retval < 0? "error":"ok");
	return (retval);
}
int gpio_usb_clear_halt (gpio_device * dev, int ep)
{
	int retval;

        if (!dev->ops->clear_halt) {
		gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level,
			"gpio_usb_clear_halt: clear_halt NULL");
                return (GPIO_ERROR);
	}

        retval = dev->ops->clear_halt(dev, ep);
	gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level,
		"gpio_usb_clear_halt: clear_halt %s", retval < 0? "error":"ok");
	return (retval);
}

int gpio_usb_msg_write (gpio_device * dev, int value, char *bytes, int size)
{
	int retval;

        if (!dev->ops->msg_write) {
		gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level,
			"gpio_usb_msg_write: msg_write NULL");
                return (GPIO_ERROR);
 	}

        retval = dev->ops->msg_write(dev, value, bytes, size);
	gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level,
		"gpio_usb_msg_write: msg_write %s", retval < 0? "error":"ok");
	return (retval);
}

int gpio_usb_msg_read (gpio_device * dev, int value, char *bytes, int size)
{
	int retval; 

        if (!dev->ops->msg_read) {
		gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level,
			"gpio_usb_msg_read: msg_read NULL");
                return (GPIO_ERROR);
	}

        retval = dev->ops->msg_read(dev, value, bytes, size);
	gpio_debug_printf(GPIO_DEBUG_LOW, dev->debug_level,
		"gpio_usb_msg_read: msg_read %s", retval < 0? "error":"ok");
	return (retval);
}
#endif
