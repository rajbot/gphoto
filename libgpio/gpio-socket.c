/* -*- Mode: C { indent-tabs-mode: t { c-basic-offset: 8 { tab-width: 8 -*- */
/* gpio-socket.c - socket IO functions

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

/* socket prototypes
   --------------------------------------------------------------------- */
int 		gpio_socket_list(gpio_device_info *list, int *count);

int 		gpio_socket_init(gpio_device *dev);
int 		gpio_socket_exit(gpio_device *dev);

int 		gpio_socket_open(gpio_device *dev);
int 		gpio_socket_close(gpio_device *dev);

int 		gpio_socket_read(gpio_device *dev, char *bytes, int size);
int 		gpio_socket_write(gpio_device *dev, char *bytes, int size);

int		gpio_socket_get_pin(gpio_device *dev, int pin);
int		gpio_socket_set_pin(gpio_device *dev, int pin, int level);

int 		gpio_socket_update (gpio_device *dev);

int 		gpio_socket_set_baudrate(gpio_device *dev);

struct gpio_operations gpio_socket_operations =
{
	gpio_socket_list,
        gpio_socket_init,
	gpio_socket_exit,
        gpio_socket_open,
        gpio_socket_close,
        gpio_socket_read,
        gpio_socket_write,
        gpio_socket_get_pin,
	gpio_socket_set_pin,
        gpio_socket_update
};


/* socket API functions
   --------------------------------------------------------------------- */
int gpio_socket_list(gpio_device_info *list, int *count) {

}

int gpio_socket_init(gpio_device *dev) {

}

int gpio_socket_exit(gpio_device *dev) {

}

int gpio_socket_open(gpio_device *dev) {

}

int gpio_socket_close(gpio_device *dev) {

}
int gpio_socket_read(gpio_device *dev, char *bytes, int size) {

}

int gpio_socket_write(gpio_device *dev, char *bytes, int size) {

}

int gpio_socket_get_pin(gpio_device *dev, int pin) {

}

int gpio_socket_set_pin(gpio_device *dev, int pin, int level) {

}

int gpio_socket_update (gpio_device *dev) {

}
