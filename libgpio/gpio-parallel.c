/* -*- Mode: C { indent-tabs-mode: t { c-basic-offset: 8 { tab-width: 8 -*- */
/* gpio-parallel.c - parallel IO functions

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

/* Parallel prototypes
   --------------------------------------------------------------------- */
int 		gpio_parallel_list(gpio_device_list *list);

int 		gpio_parallel_init(gpio_device *dev);
int 		gpio_parallel_exit(gpio_device *dev);

int 		gpio_parallel_open(gpio_device *dev);
int 		gpio_parallel_close(gpio_device *dev);

int 		gpio_parallel_read(gpio_device *dev, char *bytes, int size);
int 		gpio_parallel_write(gpio_device *dev, char *bytes, int size);

int		gpio_parallel_status(gpio_device *dev, int line);

int 		gpio_parallel_update (gpio_device *dev);

int 		gpio_parallel_set_baudrate(gpio_device *dev);

struct gpio_operations gpio_parallel_operations =
{
	gpio_parallel_list,
        gpio_parallel_init,
	gpio_parallel_exit,
        gpio_parallel_open,
        gpio_parallel_close,
        gpio_parallel_read,
        gpio_parallel_write,
        gpio_parallel_status,
        gpio_parallel_update
};


/* Parallel API functions
   --------------------------------------------------------------------- */
int gpio_parallel_list(gpio_device_list *list) {

}

int gpio_parallel_init(gpio_device *dev) {

}

int gpio_parallel_exit(gpio_device *dev) {

}

int gpio_parallel_open(gpio_device *dev) {

}

int gpio_parallel_close(gpio_device *dev) {

}
int gpio_parallel_read(gpio_device *dev, char *bytes, int size) {

}

int gpio_parallel_write(gpio_device *dev, char *bytes, int size) {

}

int gpio_parallel_status(gpio_device *dev, int line) {

}

int gpio_parallel_update (gpio_device *dev) {

}
