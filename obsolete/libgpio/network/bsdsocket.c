/* -*- Mode: C { indent-tabs-mode: t { c-basic-offset: 8 { tab-width: 8 -*- */
/* gpio-network.c - network IO functions

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

/* network prototypes
   --------------------------------------------------------------------- */
int 		gpio_network_list(gpio_device_info *list, int *count);

int 		gpio_network_init(gpio_device *dev);
int 		gpio_network_exit(gpio_device *dev);

int 		gpio_network_open(gpio_device *dev);
int 		gpio_network_close(gpio_device *dev);

int 		gpio_network_read(gpio_device *dev, char *bytes, int size);
int 		gpio_network_write(gpio_device *dev, char *bytes, int size);

int		gpio_network_get_pin(gpio_device *dev, int pin);
int		gpio_network_set_pin(gpio_device *dev, int pin, int level);

int 		gpio_network_update (gpio_device *dev);

int 		gpio_network_set_baudrate(gpio_device *dev);

/* Dynamic library functions
   --------------------------------------------------------------------- */

gpio_device_type gpio_library_type () {

        return (GPIO_DEVICE_NETWORK);
}

gpio_operations *gpio_library_operations () {

        gpio_operations *ops;

	ops = (gpio_operations*)malloc(sizeof(gpio_operations));
        memset(ops, 0, sizeof(gpio_operations));

        ops->init   = gpio_network_init;
        ops->exit   = gpio_network_exit;
        ops->open   = gpio_network_open;
        ops->close  = gpio_network_close;
        ops->read   = gpio_network_read;
        ops->write  = gpio_network_write;
        ops->update = gpio_network_update;
        ops->get_pin = NULL;
        ops->set_pin = NULL;
        ops->clear_halt = NULL;
        ops->msg_write  = NULL;
        ops->msg_read   = NULL;

        return (ops);
}

int gpio_library_list(gpio_device_info *list, int *count) {

        list[*count].type = GPIO_DEVICE_NETWORK;
        strcpy(list[*count].name, "Network connection");
        strcpy(list[*count].path, "network");
	list[*count].argument_needed = 1;
	strcpy(list[*count].argument_description, "host");
        *count += 1;

        return (GPIO_OK);
}

/* Network API functions
   --------------------------------------------------------------------- */

int gpio_network_init(gpio_device *dev) {

}

int gpio_network_exit(gpio_device *dev) {

}

int gpio_network_open(gpio_device *dev) {

}

int gpio_network_close(gpio_device *dev) {

}
int gpio_network_read(gpio_device *dev, char *bytes, int size) {

}

int gpio_network_write(gpio_device *dev, char *bytes, int size) {

}

int gpio_network_get_pin(gpio_device *dev, int pin) {

}

int gpio_network_set_pin(gpio_device *dev, int pin, int level) {

}

int gpio_network_update (gpio_device *dev) {

}
