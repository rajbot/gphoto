
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
)   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GPIO Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>
#include <string.h>
#include "gpio.h"

gpio_device_info device_list[256];
int		 device_count;

/*
   Core IO library functions
   ----------------------------------------------------------------
 */

int gpio_init() {

	/* Enumerate all the available devices */
	device_count=0;

	gpio_serial_operations.list(device_list, &device_count);
	gpio_parallel_operations.list(device_list, &device_count);
#ifdef GPIO_SOCKET
	gpio_socket_operations.list(device_list, &device_count);
#endif
#ifdef GPIO_USB
	gpio_usb_operations.list(device_list, &device_count);
#endif
#ifdef GPIO_IEEE1394
	gpio_ieee1394_operations.list(device_list, &device_count);
#endif

	return (GPIO_OK);

}

int gpio_get_device_count() {

	return (device_count);
}
int gpio_get_device_info(int device_number, gpio_device_info *info) {

	memcpy(info, &device_list[device_number], sizeof(device_list[device_number]));

	return (GPIO_OK);
}

gpio_device *gpio_new(gpio_device_type type)
	/* Create a new IO device */
{
	gpio_device *dev;
	gpio_device_settings settings;
	char buf[1024];

	dev = (gpio_device *) malloc(sizeof(gpio_device));
	dev->type = type;
	dev->device_fd = 0;

	switch (dev->type) {
	case GPIO_DEVICE_SERIAL:
		dev->ops = &gpio_serial_operations;
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
		dev->ops = &gpio_parallel_operations;
		sprintf(buf, GPIO_SERIAL_PREFIX, GPIO_SERIAL_RANGE_LOW);
	        strcpy(settings.parallel.port, buf);
		break;
#ifdef GPIO_SOCKET
	case GPIO_DEVICE_SOCKET:
		dev->ops = &gpio_socket_operations;
		gpio_set_timeout(dev, 50000);
		break;
#endif
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

gpio_device *gpio_new_by_number (int device_number) {

	gpio_device *dev;

	if (device_number >= device_count)
		return (NULL);

	dev = gpio_new(device_list[device_number].type);

	/* apply the enumerated device settings */
	switch (dev->type) {
		case GPIO_DEVICE_SERIAL:
		        strcpy(dev->settings.serial.port, device_list[device_number].path);
			break;

		case GPIO_DEVICE_PARALLEL:
		        strcpy(dev->settings.parallel.port, device_list[device_number].path);
			break;
		case GPIO_DEVICE_SOCKET:
			/* uhhh. don't do anything */
			break;
#ifdef GPIO_USB
		case GPIO_DEVICE_USB:
			/* set endpoints? or what? */
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
	return (dev);

}

gpio_device *gpio_new_by_path (char *path) {

	int x=0;

	while (x<device_count) {
		if (strcmp(device_list[x].path, path)==0)
			return(gpio_new_by_number(x));
		x++;
	}

	return (NULL);
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
	if (dev->type == GPIO_DEVICE_SERIAL && dev->device_fd == 0)
		return GPIO_OK;

	retval = dev->ops->close(dev);
	dev->device_fd = 0;

	return retval;
}

int gpio_free(gpio_device * dev)
	/* Frees a device struct */
{
	int retval = dev->ops->exit(dev);	
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

int gpio_get_pin(gpio_device * dev, int pin)
{
	/* Give the status of pin from dev */
	return dev->ops->get_pin(dev, pin);
}
int gpio_set_pin(gpio_device * dev, int pin, int level)
{
	/* Set the status of pin from dev to level */
	return dev->ops->set_pin(dev, pin, level);
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
