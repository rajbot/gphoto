/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gpio.c - Core IO library functions

   Modifications:
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

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gpio.h"

gpio_device_info device_list[256];
int		 device_count;

/* Toggle to turn on/off debugging */
int		 device_debug=0;

void gpio_debug_printf (char *format, ...) {

        va_list         pvar;

        if (device_debug) {
                fprintf(stderr, "gpio: ");
                va_start(pvar, format);
                vfprintf(stderr, format, pvar);
                va_end(pvar);
                fprintf(stderr, "\n");
        }
}

/*
   Required library functions
   ----------------------------------------------------------------
 */

int gpio_init()
{
	gpio_operations *ops;
	/* Enumerate all the available devices */
	device_count = 0;

	return (gpio_library_list(device_list, &device_count));
}

int gpio_get_device_count(void)
{
	return device_count;
}

int gpio_get_device_info(int device_number, gpio_device_info *info)
{
	memcpy(info, &device_list[device_number], sizeof(device_list[device_number]));

	return GPIO_OK;
}

gpio_device *gpio_new(gpio_device_type type)
	/* Create a new IO device */
{
	gpio_device *dev;
	gpio_device_settings settings;
	char buf[1024];

	dev = (gpio_device *) malloc(sizeof(gpio_device));
	if (!dev) {
		fprintf(stderr, "unable to allocate memory for gpio device\n");
		return NULL;
	}

	if (gpio_library_load(dev, type)) {
		/* whoops! that type of device isn't supported */
		free(dev);
		return NULL;
	}

	dev->type = type;
	dev->device_fd = 0;

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
		gpio_set_timeout(dev, 5000);
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

	dev->ops->init(dev);

	return (dev);
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
			return GPIO_ERROR;
		}
		return GPIO_OK;
	}
	return GPIO_ERROR;
}

int gpio_close(gpio_device *dev)
	/* Close the device to prevent reading/writing */
{
	int retval = 0;

	if (!dev)
		return GPIO_ERROR;
	if (dev->type == GPIO_DEVICE_SERIAL && dev->device_fd == 0)
		return GPIO_OK;

	retval = dev->ops->close(dev);
	dev->device_fd = 0;

	return retval;
}

int gpio_free(gpio_device *dev)
	/* Frees a device struct */
{
	int retval = dev->ops->exit(dev);	

	gpio_library_close(dev);

	free(dev);

	return GPIO_OK;
}

int gpio_write(gpio_device *dev, char *bytes, int size)
	/* Called to write "bytes" to the IO device */
{
	return dev->ops->write(dev, bytes, size);
}

int gpio_read(gpio_device *dev, char *bytes, int size)
	/* Reads data from the device into the "bytes" buffer.
	   "bytes" should be large enough to hold all the data.
	 */
{
	return dev->ops->read(dev, bytes, size);
}

int gpio_set_timeout(gpio_device *dev, int millisec_timeout)
{
	dev->timeout = millisec_timeout;
	return GPIO_OK;
}

int gpio_get_timeout(gpio_device *dev, int *millisec_timeout)
{
	*millisec_timeout = dev->timeout;
	return GPIO_OK;
}

int gpio_set_settings(gpio_device *dev, gpio_device_settings settings)
{
	/* need to memcpy() settings to dev->settings */
	memcpy(&dev->settings_pending, &settings, sizeof(dev->settings_pending));

	return dev->ops->update(dev);
}


int gpio_get_settings(gpio_device *dev, gpio_device_settings * settings)
{
	memcpy(settings, &dev->settings, sizeof(gpio_device_settings));
	return GPIO_OK;
}

/* Serial and Parallel-specific functions */
/* ------------------------------------------------------------------ */

int gpio_get_pin(gpio_device *dev, int pin)
{
	if (!dev->ops->get_pin)
		return (GPIO_ERROR);

	return (dev->ops->get_pin(dev, pin));
}

int gpio_set_pin(gpio_device *dev, int pin, int level)
{
	if (!dev->ops->get_pin)
		return (GPIO_ERROR);

	return (dev->ops->set_pin(dev, pin, level));
}


/* USB-specific functions */
/* ------------------------------------------------------------------ */

#ifdef GPIO_USB

int gpio_usb_find_device (gpio_device * dev, int idvendor, int idproduct)
{
	if (!dev->ops->find_device)
		return (GPIO_ERROR);

	return (dev->ops->find_device(dev, idvendor, idproduct));
}
int gpio_usb_clear_halt (gpio_device * dev)
{
	if (!dev->ops->clear_halt)
		return (GPIO_ERROR);

	return (dev->ops->clear_halt(dev));
}

int gpio_usb_msg_write (gpio_device * dev, int value, char *bytes, int size)
{
	if (!dev->ops->msg_write)
		return (GPIO_ERROR);

	return (dev->ops->msg_write(dev, value, bytes, size));
}

int gpio_usb_msg_read (gpio_device * dev, int value, char *bytes, int size)
{
	if (!dev->ops->msg_read)
		return (GPIO_ERROR);

	return (dev->ops->msg_read(dev, value, bytes, size));
}
#endif
