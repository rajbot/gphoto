
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

#include "gpio.h"

gpio_device_list device_list[256];

/*
   Core IO library functions
   ----------------------------------------------------------------
 */

int gpio_init(void)
{
#ifdef GPIO_USB
	usb_init();
	usb_find_busses();
	usb_find_devices();
#endif
	return (GPIO_OK);
}

gpio_device *gpio_new(int device_number)
	/* Create a new IO device */
{
	gpio_device *dev;
	gpio_device_settings settings;

	dev = (gpio_device *) malloc(sizeof(gpio_device));
	dev->type = device_list[device_number].type;
	dev->device_fd = 0;

	switch (dev->type) {
	case GPIO_DEVICE_SERIAL:
		dev->ops = &gpio_serial_operations;
	        strcpy(settings.serial.port, device_list[device_number].path);
		/* set some defaults */
        	settings.serial.speed = 9600;
        	settings.serial.bits = 8;
        	settings.serial.parity = 0;
        	settings.serial.stopbits = 1;
		gpio_set_settings(dev, settings);
		gpio_set_timeout(dev, 5000);
		break;
	case GPIO_DEVICE_PARALLEL:
		dev->ops = &gpio_parallel_operations;
	        strcpy(settings.parallel.port, device_list[device_number].path);
		break;
#ifdef GPIO_USB
	case GPIO_DEVICE_USB:
		dev->ops = &gpio_usb_operations;
		gpio_set_timeout(dev, 5000);
		break;
#endif
#ifdef GPIO_IEEE1394
	case GPIO_DEVICE_1394:
		break;
#endif
	default:
		// ERROR!
		break;
	}

	dev->ops->init(dev);

	return (dev);
}

int gpio_open(gpio_device * dev)
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

int gpio_close(gpio_device * dev)
	/* Close the device to prevent reading/writing */
{
	int retval = 0;

	if (!dev)
		return GPIO_ERROR;
	if (dev->device_fd == 0)
		return GPIO_OK;

	retval = dev->ops->close(dev);
	dev->device_fd = 0;

	return retval;
}

int gpio_free(gpio_device * dev)
	/* Frees a device struct */
{
	free(dev);

	return GPIO_OK;
}

int gpio_write(gpio_device * dev, char *bytes, int size)
	/* Called to write "bytes" to the IO device */
{
	return dev->ops->write(dev, bytes, size);
}

int gpio_read(gpio_device * dev, char *bytes, int size)
	/* Reads data from the device into the "bytes" buffer.
	   "bytes" should be large enough to hold all the data.
	 */
{
	return dev->ops->read(dev, bytes, size);
}

int gpio_status(gpio_device * dev, int line)
{
	/* Give the status of line from dev */
	return dev->ops->status(dev, line);
}
int gpio_set_timeout(gpio_device * dev, int millisec_timeout)
{
	dev->timeout = millisec_timeout;
	return GPIO_OK;
}

int gpio_get_timeout(gpio_device * dev, int *millisec_timeout)
{
	*millisec_timeout = dev->timeout;
	return GPIO_OK;
}

int gpio_set_settings(gpio_device * dev, gpio_device_settings settings)
{
	/* need to memcpy() settings to dev->settings */
	memcpy(&dev->settings_pending, &settings, sizeof(dev->settings_pending));

	return dev->ops->update(dev);
}


int gpio_get_settings(gpio_device * dev, gpio_device_settings * settings)
{
	memcpy(settings, &dev->settings, sizeof(gpio_device_settings));
	return GPIO_OK;
}
