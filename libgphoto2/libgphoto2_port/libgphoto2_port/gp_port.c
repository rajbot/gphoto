/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * Copyright (C) 2001 Lutz M�ller <urc8@rz.uni-karlsruhe.de>
 * Copyright (C) 1999 Scott Fritzinger <scottf@unr.edu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details. 
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include "gphoto2-port.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gphoto2-port-result.h"
#include "gphoto2-port-info-list.h"
#include "gphoto2-port-library.h"
#include "gphoto2-port-log.h"

#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#      define N_(String) gettext_noop (String)
#  else
#      define N_(String) (String)
#  endif
#else
#  define _(String) (String)
#  define N_(String) (String)
#endif

#define CHECK_RESULT(result) {int r=(result); if (r<0) return (r);}
#define CHECK_NULL(m) {if (!(m)) return (GP_ERROR_BAD_PARAMETERS);}
#define CHECK_SUPP(s,o) {if (!(o)) {gp_log (GP_LOG_ERROR, "gphoto2-port", ("The operation '%s' is not supported by this device"), (s)); return (GP_ERROR_NOT_SUPPORTED);}}

struct _GPPortPrivateCore {
	char error[2048];

	void *lh; /* Library handle */
};

static int
gp_port_library_load (GPPort *device, GPPortType type)
{
	GPPortInfo info;
	int count, result, i;
	GPPortInfoList *list;
        GPPortLibraryOperations ops_func;

	CHECK_RESULT (gp_port_info_list_new (&list));
	result = gp_port_info_list_load (list);
	if (result < 0) {
		gp_port_info_list_free (list);
		return (result);
	}
	count = gp_port_info_list_count (list);
	if (count < 0) {
		gp_port_info_list_free (list);
		return (count);
	}
	for (i = 0; i < count; i++) {
		gp_port_info_list_get_info (list, i, &info);
		if (info.type == type)
			break;
	}
	gp_port_info_list_free (list);
	if (i == count)
		return (GP_ERROR_UNKNOWN_PORT);

	/* Open the correct library */
	device->pc->lh = GP_SYSTEM_DLOPEN (info.library_filename);
	if (!device->pc->lh) {
		gp_log (GP_LOG_ERROR, "gphoto2-port", "Could not load "
			"'%s' ('%s')", info.library_filename,
			GP_SYSTEM_DLERROR ());
		return (GP_ERROR_LIBRARY);
	}

	/* Load the operations */
	ops_func = GP_SYSTEM_DLSYM (device->pc->lh,
				    "gp_port_library_operations");
	if (!ops_func) {
		gp_log (GP_LOG_ERROR, "gphoto2-port", "Could not find "
			"'gp_port_library_operations' in '%s' ('%s')",
			info.library_filename, GP_SYSTEM_DLERROR ());
		GP_SYSTEM_DLCLOSE (device->pc->lh);
		device->pc->lh = NULL;
		return (GP_ERROR_LIBRARY);
	}
	device->ops = ops_func ();
	
	return (GP_OK);
}

int
gp_port_new (GPPort **dev, GPPortType type)
{
	GPPortSettings settings;
        char buf[1024];
	int result;

	CHECK_NULL (dev);

        gp_log (GP_LOG_DEBUG, "gphoto2-port", "Creating new device...");

        switch(type) {
        case GP_PORT_SERIAL:
#ifndef GP_PORT_SUPPORTED_SERIAL
		return GP_ERROR_IO_SUPPORTED_SERIAL;
#endif
		break;
        case GP_PORT_USB:
#ifndef GP_PORT_SUPPORTED_USB
		gp_log (GP_LOG_ERROR, "gphoto2-port", _("libgphoto2_port "
			"has been compiled without USB support. This probably "
			"means you didn't have libusb installed prior to "
			"compilation of gphoto2."));
		return GP_ERROR_IO_SUPPORTED_USB;
#endif
		break;
        case GP_PORT_PARALLEL:
#ifndef GP_PORT_SUPPORTED_PARALLEL
		return GP_ERROR_IO_SUPPORTED_PARALLEL;
#endif
		break;
        case GP_PORT_NETWORK:
#ifndef GP_PORT_SUPPORTED_NETWORK
		return GP_ERROR_IO_SUPPORTED_NETWORK;
#endif
		break;
        case GP_PORT_IEEE1394:
#ifndef GP_PORT_SUPPORTED_IEEE1394
		return GP_ERROR_IO_SUPPORTED_IEEE1394;
#endif
		break;
        default:
		return GP_ERROR_UNKNOWN_PORT;
        }

        *dev = malloc (sizeof (GPPort));
        if (!(*dev))
		return (GP_ERROR_NO_MEMORY);
        memset (*dev, 0, sizeof (GPPort));

	(*dev)->pc = malloc (sizeof (GPPortPrivateCore));
	if (!(*dev)->pc) {
		gp_port_free (*dev);
		return (GP_ERROR_NO_MEMORY);
	}
	memset ((*dev)->pc, 0, sizeof (GPPortPrivateCore));

	result = gp_port_library_load (*dev, type);
	if (result < 0) {
		gp_port_free (*dev);
		return (result);
        }

        (*dev)->type = type;
        (*dev)->device_fd = 0;
        (*dev)->ops->init (*dev);

        switch ((*dev)->type) {
        case GP_PORT_SERIAL:
		sprintf(buf, GP_PORT_SERIAL_PREFIX, GP_PORT_SERIAL_RANGE_LOW);
		strcpy(settings.serial.port, buf);

		/* Set some default settings */
		settings.serial.speed = 0;
		settings.serial.bits = 8;
		settings.serial.parity = 0;
		settings.serial.stopbits = 1;
		gp_port_settings_set (*dev, settings);

		gp_port_timeout_set (*dev, 500);
		break;
        case GP_PORT_PARALLEL:
		sprintf (buf, GP_PORT_SERIAL_PREFIX, GP_PORT_SERIAL_RANGE_LOW);
		strcpy (settings.parallel.port, buf);
		break;
        case GP_PORT_NETWORK:
		gp_port_timeout_set (*dev, 50000);
		break;
        case GP_PORT_USB:
		/* Initialize settings.usb */
		(*dev)->settings.usb.inep = -1;
		(*dev)->settings.usb.outep = -1;
		(*dev)->settings.usb.config = -1;
		(*dev)->settings.usb.interface = 0;
		(*dev)->settings.usb.altsetting = -1;

		gp_port_timeout_set (*dev, 5000);
		break;
        case GP_PORT_IEEE1394:
		/* blah ? */
		break;
        default:
		return GP_ERROR_UNKNOWN_PORT;
        }

        return (GP_OK);
}

int
gp_port_open (GPPort *dev)
{
	CHECK_NULL (dev);

	gp_log (GP_LOG_DEBUG, "gphoto2-port", "Opening %s port...",
		dev->type == GP_PORT_SERIAL ? "SERIAL" : 
			(dev->type == GP_PORT_USB ? "USB" : ""));

	CHECK_SUPP (_("open"), dev->ops->open);
	CHECK_RESULT (dev->ops->open (dev));

	return GP_OK;
}

int
gp_port_close (GPPort *dev)
{
	gp_log (GP_LOG_DEBUG, "gphoto2-port", "Closing port...");

	CHECK_NULL (dev);

	CHECK_SUPP (_("close"), dev->ops->close);
        CHECK_RESULT (dev->ops->close(dev));

	return (GP_OK);
}

int
gp_port_free (GPPort *dev)
{
	gp_log (GP_LOG_DEBUG, "gphoto2-port", "Freeing port...");

	CHECK_NULL (dev);

	if (dev->ops) {

		/* We don't care if this operation is unsupported or fails */
		if (dev->ops->exit)
			dev->ops->exit(dev);
		free (dev->ops);
		dev->ops = NULL;
	}

	if (dev->pc) {
		if (dev->pc->lh) {
		        GP_SYSTEM_DLCLOSE (dev->pc->lh);
			dev->pc->lh = NULL;
		}
		free (dev->pc);
		dev->pc = NULL;
	}

        free (dev);

        return GP_OK;
}

int
gp_port_write (GPPort *dev, char *data, int size)
{
	int retval;

	gp_log (GP_LOG_DEBUG, "gphoto2-port", "Writing %i byte(s) to port...",
		size);

	CHECK_NULL (dev && data);

	gp_log_data ("gphoto2-port", data, size);

	/* Check if we wrote all bytes */
	CHECK_SUPP (_("write"), dev->ops->write);
	retval = dev->ops->write (dev, data, size);
	CHECK_RESULT (retval);
	if ((dev->type != GP_PORT_SERIAL) && (retval != size))
		gp_log (GP_LOG_DEBUG, "gphoto2-port", "Could only write %i "
			"out of %i byte(s)", retval, size);

	return (retval);
}

int
gp_port_read (GPPort *dev, char *data, int size)
{
        int retval;

	gp_log (GP_LOG_DEBUG, "gphoto2-port", "Reading from port...");

	CHECK_NULL (dev);

	/* Check if we read as many bytes as expected */
	CHECK_SUPP (_("read"), dev->ops->read);
	retval = dev->ops->read (dev, data, size);
	CHECK_RESULT (retval);
	if (retval != size)
		gp_log (GP_LOG_DEBUG, "gphoto2-port", "Could only read %i "
			"out of %i byte(s)", retval, size);

	gp_log_data ("gphoto2-port", data, retval);

	return (retval);
}

int
gp_port_timeout_set (GPPort *dev, int timeout)
{
	gp_log (GP_LOG_DEBUG, "gphoto2-port", "Setting timeout to %i "
		"millisecond(s)...", timeout);

	CHECK_NULL (dev);

        dev->timeout = timeout;

        return GP_OK;
}

int
gp_port_timeout_get (GPPort *dev, int *timeout)
{
	gp_log (GP_LOG_DEBUG, "gphoto2-port", "Getting timeout...");

	CHECK_NULL (dev);

	gp_log (GP_LOG_DEBUG, "gphoto2-port", "Current timeout: %i "
		"milliseconds", dev->timeout);

        *timeout = dev->timeout;

        return GP_OK;
}

int
gp_port_settings_set (GPPort *dev, gp_port_settings settings)
{
	gp_log (GP_LOG_DEBUG, "gphoto2-port", "Setting settings...");

	CHECK_NULL (dev);

        /*
	 * We copy the settings to settings_pending and call update on the 
	 * port.
	 */
        memcpy (&dev->settings_pending, &settings,
		sizeof (dev->settings_pending));
	CHECK_SUPP (_("update"), dev->ops->update);
        CHECK_RESULT (dev->ops->update (dev));

        return (GP_OK);
}

int
gp_port_settings_get (GPPort *dev, gp_port_settings * settings)
{
	CHECK_NULL (dev);

        memcpy (settings, &(dev->settings), sizeof (gp_port_settings));

        return GP_OK;
}

int
gp_port_pin_get(GPPort *dev, int pin, int *level)
{
	gp_log (GP_LOG_DEBUG, "gphoto2-port", "Getting level of pin %i...",
		pin);

	CHECK_NULL (dev && level);

	CHECK_SUPP (_("get_pin"), dev->ops->get_pin);
        CHECK_RESULT (dev->ops->get_pin (dev, pin, level));

	gp_log (GP_LOG_DEBUG, "gphoto2-port", "Level of pin %i: %i",
		pin, *level);

	return (GP_OK);
}

int
gp_port_pin_set (GPPort *dev, int pin, int level)
{
	gp_log (GP_LOG_DEBUG, "gphoto2-port", "Setting pin %i to %i...",
		pin, level);

	CHECK_NULL (dev);

	CHECK_SUPP (_("set_pin"), dev->ops->set_pin);
	CHECK_RESULT (dev->ops->set_pin (dev, pin, level));

	return (GP_OK);
}

int
gp_port_send_break (GPPort *dev, int duration)
{
	gp_log (GP_LOG_DEBUG, "gphoto2-port", "Sending break (%i "
		"milliseconds)...", duration);

	CHECK_NULL (dev);

        CHECK_SUPP (_("send_break"), dev->ops->send_break);
        CHECK_RESULT (dev->ops->send_break (dev, duration));

	return (GP_OK);
}

int
gp_port_flush (GPPort *dev, int direction)
{
	gp_log (GP_LOG_DEBUG, "gphoto2-port", "Flushing port...");

	CHECK_NULL (dev);

	CHECK_SUPP (_("flush"), dev->ops->flush);
	CHECK_RESULT (dev->ops->flush (dev, direction));

        return (GP_OK);
}


/* USB-specific functions */
/* ------------------------------------------------------------------ */

int gp_port_usb_find_device (GPPort *dev, int idvendor, int idproduct)
{
	gp_log (GP_LOG_DEBUG, "gphoto2-port", "Looking for (0x%x, 0x%x)...",
		idvendor, idproduct);

	CHECK_NULL (dev);

	CHECK_SUPP (_("find_device"), dev->ops->find_device);
	CHECK_RESULT (dev->ops->find_device (dev, idvendor, idproduct));

        return (GP_OK);
}

int gp_port_usb_clear_halt (GPPort *dev, int ep)
{
	gp_log (GP_LOG_DEBUG, "gphoto2-port", "Clear halt...");

	CHECK_NULL (dev);

	CHECK_SUPP (_("clear_halt"), dev->ops->clear_halt);
        CHECK_RESULT (dev->ops->clear_halt (dev, ep));

        return (GP_OK);
}

int gp_port_usb_msg_write (GPPort *dev, int request, int value, int index,
	char *bytes, int size)
{
        int retval;

	gp_log (GP_LOG_DEBUG, "gphoto2-port", "Writing message "
		"(request=0x%x value=0x%x index=0x%x size=%i=0x%x)...",
		request, value, index, size, size);
	gp_log_data ("gphoto2-port", bytes, size);

	CHECK_NULL (dev);

	CHECK_SUPP (_("msg_write"), dev->ops->msg_write);
        retval = dev->ops->msg_write(dev, request, value, index, bytes, size);
	CHECK_RESULT (retval);

        return (retval);
}

int gp_port_usb_msg_read (GPPort *dev, int request, int value, int index,
	char *bytes, int size)
{
        int retval;

	gp_log (GP_LOG_DEBUG, "gphoto2-port", "Reading message "
		"(request=0x%x value=0x%x index=0x%x size=%i=0x%x)...",
		request, value, index, size, size);

	CHECK_NULL (dev);

	CHECK_SUPP (_("msg_read"), dev->ops->msg_read);
        retval = dev->ops->msg_read (dev, request, value, index, bytes, size);
	CHECK_RESULT (retval);

	gp_log (GP_LOG_DEBUG, "gphoto2-port", bytes, retval);

        return (retval);
}

/**
 * gp_port_set_error:
 * @port: a #GPPort
 * @format:
 * ...:
 *
 * Sets an error message that can later be retrieved using #gp_port_get_error.
 *
 * Return value: a gphoto2 error code
 **/
int
gp_port_set_error (GPPort *port, const char *format, ...)
{
	va_list args;

	if (!port)
		return (GP_ERROR_BAD_PARAMETERS);

	if (format) {
		va_start (args, format);
		vsnprintf (port->pc->error, sizeof (port->pc->error),
			   format, args);
		gp_logv (GP_LOG_ERROR, "gphoto2-port", format, args);
		va_end (args);
	} else
		port->pc->error[0] = '\0';

	return (GP_OK);
}

/**
 * gp_port_get_error:
 * @port: a #GPPort
 *
 * Retrieves an error message from a @port. If you want to make sure that
 * you get correct error messages, you need to call #gp_port_set_error with
 * an error message of %NULL each time before calling another port-related
 * function of which you want to check the return value.
 *
 * Return value: an error message
 **/
const char *
gp_port_get_error (GPPort *port)
{
	if (port && port->pc && strlen (port->pc->error))
		return (port->pc->error);

	return (N_("No error description available"));
}
