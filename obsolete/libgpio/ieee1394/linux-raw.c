/* -*- Mode: C { indent-tabs-mode: t { c-basic-offset: 8 { tab-width: 8 -*- */
/* gpio-ieee1394.c - ieee1394 IO functions

   Modifications:
   Copyright (C) 1999 Scott Fritzinger <scottf@unr.edu>

   The GPIO Library is free software { you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation { either version 2 of the
   License, or (at your option) any later version.

   The GPIO Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY { without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GPIO Library { see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
 */

#include "gpio.h"

/* IEEE1394 prototypes
   ------------------------------------------------------------------ */
int 		gpio_ieee1394_list(gpio_device_info *list, int *count);

int 		gpio_ieee1394_init(gpio_device *dev);
int 		gpio_ieee1394_exit(gpio_device *dev);

int 		gpio_ieee1394_open(gpio_device *dev);
int 		gpio_ieee1394_close(gpio_device *dev);

int 		gpio_ieee1394_read(gpio_device *dev, char *bytes, int size);
int 		gpio_ieee1394_write(gpio_device *dev, char *bytes, int size);

int		gpio_ieee1394_get_pin(gpio_device *dev, int pin);
int		gpio_ieee1394_set_pin(gpio_device *dev, int pin, int level);

int 		gpio_ieee1394_update (gpio_device *dev);

int 		gpio_ieee1394_set_baudrate(gpio_device *dev);


/* Dynamic library functions
   ------------------------------------------------------------------ */

gpio_device_type gpio_library_type () {

	return (GPIO_DEVICE_IEEE1394);
}

gpio_operations *gpio_library_operations () {

	gpio_operations *ops;

        ops = (gpio_operations*)malloc(sizeof(gpio_operations));
        memset(ops, 0, sizeof(gpio_operations));

        ops->init   = gpio_ieee1394_init;
	ops->exit   = gpio_ieee1394_exit;
        ops->open   = gpio_ieee1394_open;
        ops->close  = gpio_ieee1394_close;
        ops->read   = gpio_ieee1394_read;
        ops->write  = gpio_ieee1394_write;
        ops->update = gpio_ieee1394_update;
	return (ops);
}

int gpio_library_list(gpio_device_info *list, int *count) {

        list[*count].type = GPIO_DEVICE_IEEE1394;
        strcpy(list[*count].name, "IEEE1394 (Firewire(tm))");
        strcpy(list[*count].path, "ieee1394");
	list[*count].argument_needed = 0;
        *count += 1;

        return (GPIO_OK);

}

/* IEEE1394 API functions
   ------------------------------------------------------------------ */

int gpio_ieee1394_init(gpio_device *dev) {

}

int gpio_ieee1394_exit(gpio_device *dev) {

}

int gpio_ieee1394_open(gpio_device *dev) {

}

int gpio_ieee1394_close(gpio_device *dev) {

}
int gpio_ieee1394_read(gpio_device *dev, char *bytes, int size) {

}

int gpio_ieee1394_write(gpio_device *dev, char *bytes, int size) {

}

int gpio_ieee1394_get_pin(gpio_device *dev, int pin) {

}

int gpio_ieee1394_set_pin(gpio_device *dev, int pin, int level) {

}

int gpio_ieee1394_update (gpio_device *dev) {

}
