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
#include "../include/gphoto2-port.h"
#include "library.h"

gp_port_info     device_list[256];
int              device_count;

/* Toggle to turn on/off debugging */
int              glob_debug_level=0;

void gp_port_debug_printf (int target_debug_level, int debug_level, char *format, ...)
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

int gp_port_init(int debug)
{
        gp_port_debug_printf(GP_DEBUG_LOW, glob_debug_level, "Initializing...");
        /* Enumerate all the available devices */
        device_count = 0;
	glob_debug_level = debug;
        return (gp_port_library_list(device_list, &device_count));
}

int gp_port_count_get(void)
{
        gp_port_debug_printf(GP_DEBUG_LOW, glob_debug_level, "Device count: %i", device_count);
        return device_count;
}

int gp_port_info_get(int device_number, gp_port_info *info)
{
        gp_port_debug_printf(GP_DEBUG_LOW, glob_debug_level, "Getting device info...");

        memcpy(info, &device_list[device_number], sizeof(device_list[device_number]));

        return GP_OK;
}

gp_port *gp_port_new(gp_port_type type)
        /* Create a new IO device */
{
        gp_port *dev;
        gp_port_settings settings;
        char buf[1024];

        gp_port_debug_printf(GP_DEBUG_LOW, glob_debug_level, "Creating new device... ");

        dev = (gp_port *) malloc(sizeof(gp_port));
        if (!dev) {
                gp_port_debug_printf(GP_DEBUG_LOW, glob_debug_level, "Can not allocate device!");
                return NULL;
        }
	memset(dev, 0, sizeof(gp_port));

        if (gp_port_library_load(dev, type)) {
                /* whoops! that type of device isn't supported */
                gp_port_debug_printf(GP_DEBUG_LOW, glob_debug_level, "Device type not supported! (%i)", type);
                free(dev);
                return NULL;
        }

	dev->debug_level = glob_debug_level;

        dev->type = type;
        dev->device_fd = 0;
	dev->ops->init(dev);

        switch (dev->type) {
        case GP_PORT_SERIAL:
                sprintf(buf, GP_PORT_SERIAL_PREFIX, GP_PORT_SERIAL_RANGE_LOW);
                strcpy(settings.serial.port, buf);
                /* set some defaults */
                settings.serial.speed = 9600;
                settings.serial.bits = 8;
                settings.serial.parity = 0;
                settings.serial.stopbits = 1;
                gp_port_settings_set(dev, settings);
                gp_port_timeout_set(dev, 500);
                break;
        case GP_PORT_PARALLEL:
                sprintf(buf, GP_PORT_SERIAL_PREFIX, GP_PORT_SERIAL_RANGE_LOW);
                strcpy(settings.parallel.port, buf);

                break;
        case GP_PORT_NETWORK:
                gp_port_timeout_set(dev, 50000);
                break;
        case GP_PORT_USB:
                gp_port_timeout_set(dev, 5000);
                break;
        case GP_PORT_IEEE1394:
                /* blah ? */
                break;
        default:
                /* ERROR! */
                break;
        }

        gp_port_debug_printf(GP_DEBUG_LOW, glob_debug_level, "Created device successfully...");

        return (dev);
}

int gp_port_debug_set (gp_port *dev, int debug_level)
{
	dev->debug_level = debug_level;

	return (GP_OK);
}

int gp_port_open(gp_port *dev)
        /* Open a device for reading/writing */
{
        int retval = 0;

        /* Try to open device */
        retval = dev->ops->open(dev);
        if (retval == GP_OK) {
                /* Now update the settings */
                retval = dev->ops->update(dev);
                if (retval != GP_OK) {
                        dev->device_fd = 0;
			gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level, 
				"gp_port_open: update error");
                        return GP_ERROR;
                }
		gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level, "gp_port_open: OK");
                return GP_OK;
        }
	gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level, "gp_port_open: open error");
        return GP_ERROR;
}

int gp_port_close(gp_port *dev)
        /* Close the device to prevent reading/writing */
{
        int retval = 0;

        if (!dev) {
		gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level, "gp_port_close: bad device");
                return GP_ERROR;
	}
        if (dev->type == GP_PORT_SERIAL && dev->device_fd == 0) {
		gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level, "gp_port_close: OK");
                return GP_OK;
	}

        retval = dev->ops->close(dev);
        dev->device_fd = 0;
	gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level, 
		"gp_port_close: close %s", retval == GP_OK? "ok":"error");
        return retval;
}

int gp_port_free(gp_port *dev)
        /* Frees a device struct */
{
        int retval = dev->ops->exit(dev);

	gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level,
		"gp_port_free: exit %s", retval < 0? "error":"ok");

        gp_port_library_close(dev);
        free(dev);

        return GP_OK;
}

int gp_port_write(gp_port *dev, char *bytes, int size)
        /* Called to write "bytes" to the IO device */
{
	int x, retval;
	char t[8];
	char *buf;

	if (glob_debug_level == GP_DEBUG_HIGH) {
		buf = (char *)malloc(sizeof(char)*(4*size+64));
		buf[0] = 0;
		for (x=0; x<size; x++) {
			sprintf(t, "%02x ", (unsigned char)bytes[x]);
			strcat(buf, t);
		}
		gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level,
			"gp_port_write: (size=%05i) DATA: %s", size, buf);
		free(buf);
	}
        retval =  dev->ops->write(dev, bytes, size);

	if (retval == GP_ERROR_TIMEOUT)
		gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level, "gp_port_write: write timeout");
	if (retval == GP_ERROR)
		gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level, "gp_port_write: write error");

	return (retval);
}

int gp_port_read(gp_port *dev, char *bytes, int size)
        /* Reads data from the device into the "bytes" buffer.
           "bytes" should be large enough to hold all the data.
         */
{
	int x, retval;	
	char t[8];
	char *buf;

	retval = dev->ops->read(dev, bytes, size);

	if ((retval > 0)&&(glob_debug_level == GP_DEBUG_HIGH)) {
		buf = (char *)malloc(sizeof(char)*(4*retval+64));
		buf[0] = 0;
		for (x=0; x<retval; x++) {
			sprintf(t, "%02x ", (unsigned char)bytes[x]);
			strcat(buf, t);
		}
		gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level, 
			"gp_port_read:  (size=%05i) DATA: %s", retval, buf);
		free(buf);
	}

	if (retval == GP_ERROR_TIMEOUT)
		gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level, "gp_port_read: read timeout");
	if (retval == GP_ERROR)
		gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level, "gp_port_read: read error");

	return (retval);
}

int gp_port_timeout_set(gp_port *dev, int millisec_timeout)
{
        dev->timeout = millisec_timeout;

	gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level, 
		"gp_port_set_timeout: value=%ims", millisec_timeout);

        return GP_OK;
}

int gp_port_timeout_get(gp_port *dev, int *millisec_timeout)
{
        *millisec_timeout = dev->timeout;

	gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level,
		"gp_port_get_timeout: value=%ims", *millisec_timeout);

        return GP_OK;
}

int gp_port_settings_set(gp_port *dev, gp_port_settings settings)
{
	int retval;

        /* need to memcpy() settings to dev->settings */
        memcpy(&dev->settings_pending, &settings, sizeof(dev->settings_pending));

        retval =  dev->ops->update(dev);
	gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level,
		"gp_port_set_settings: update %s", retval < 0? "error":"ok");
	return (retval);
}


int gp_port_settings_get(gp_port *dev, gp_port_settings * settings)
{
        memcpy(settings, &dev->settings, sizeof(gp_port_settings));
	gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level, "gp_port_get_settings: ok");

        return GP_OK;
}

/* Serial and Parallel-specific functions */
/* ------------------------------------------------------------------ */

int gp_port_pin_get(gp_port *dev, int pin)
{
	int retval;

        if (!dev->ops->get_pin) {
		gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level, "gp_port_get_pin: get_pin NULL");
                return (GP_ERROR);
	}

        retval = dev->ops->get_pin(dev, pin);
	gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level, 
		"gp_port_get_pin: get_pin %s", retval < 0? "error":"ok");
	return (retval);
}

int gp_port_pin_set(gp_port *dev, int pin, int level)
{
	int retval;

        if (!dev->ops->get_pin) {
		gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level, "gp_port_set_pin: set_pin NULL");
                return (GP_ERROR);
	}

        retval = dev->ops->set_pin(dev, pin, level);
	gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level,
		"gp_port_set_pin: set_pin %s", retval < 0? "error":"ok");
	return (retval);
}

int gp_port_send_break (gp_port *dev, int duration)
{
	int retval;

        if (!dev->ops->send_break) {
		gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level, "gp_port_break: gp_port_break NULL");
                return (GP_ERROR);
	}

        retval = dev->ops->send_break(dev, duration);
	gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level,
		"gp_port_send_break: send_break %s", retval < 0? "error":"ok");
	return (retval);
}

/* USB-specific functions */
/* ------------------------------------------------------------------ */

int gp_port_usb_find_device (gp_port * dev, int idvendor, int idproduct)
{
	int retval;

        if (!dev->ops->find_device) {
		gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level,
			"gp_port_usb_find_device: find_device NULL");
                return (GP_ERROR);
	}

        retval = dev->ops->find_device(dev, idvendor, idproduct);
	gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level, 
		"gp_port_usb_find_device: find_device (0x%04x 0x%04x) %s", 
		idvendor, idproduct, retval < 0? "error":"ok");
	return (retval);
}
int gp_port_usb_clear_halt (gp_port * dev, int ep)
{
	int retval;

        if (!dev->ops->clear_halt) {
		gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level,
			"gp_port_usb_clear_halt: clear_halt NULL");
                return (GP_ERROR);
	}

        retval = dev->ops->clear_halt(dev, ep);
	gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level,
		"gp_port_usb_clear_halt: clear_halt %s", retval < 0? "error":"ok");
	return (retval);
}

int gp_port_usb_msg_write (gp_port * dev, int value, char *bytes, int size)
{
	int retval;

        if (!dev->ops->msg_write) {
		gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level,
			"gp_port_usb_msg_write: msg_write NULL");
                return (GP_ERROR);
 	}

        retval = dev->ops->msg_write(dev, value, bytes, size);
	gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level,
		"gp_port_usb_msg_write: msg_write %s", retval < 0? "error":"ok");
	return (retval);
}

int gp_port_usb_msg_read (gp_port * dev, int value, char *bytes, int size)
{
	int retval; 

        if (!dev->ops->msg_read) {
		gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level,
			"gp_port_usb_msg_read: msg_read NULL");
                return (GP_ERROR);
	}

        retval = dev->ops->msg_read(dev, value, bytes, size);
	gp_port_debug_printf(GP_DEBUG_LOW, dev->debug_level,
		"gp_port_usb_msg_read: msg_read %s", retval < 0? "error":"ok");
	return (retval);
}
