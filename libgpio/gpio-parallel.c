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

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include "gpio.h"

/* Parallel prototypes
   --------------------------------------------------------------------- */
int 		gpio_parallel_list(gpio_device_info *list, int *count);

int 		gpio_parallel_init(gpio_device *dev);
int 		gpio_parallel_exit(gpio_device *dev);

int 		gpio_parallel_open(gpio_device *dev);
int 		gpio_parallel_close(gpio_device *dev);

int 		gpio_parallel_read(gpio_device *dev, char *bytes, int size);
int 		gpio_parallel_write(gpio_device *dev, char *bytes, int size);

int		gpio_parallel_get_pin(gpio_device *dev, int pin);
int		gpio_parallel_set_pin(gpio_device *dev, int pin, int level);

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
        gpio_parallel_get_pin,
	gpio_parallel_set_pin,
        gpio_parallel_update
};


/* Parallel API functions
   --------------------------------------------------------------------- */
int gpio_parallel_list(gpio_device_info *list, int *count) {

        char buf[1024];
        int x, fd, use_int=0, use_char=0;

        for (x=GPIO_PARALLEL_RANGE_LOW; x<=GPIO_PARALLEL_RANGE_HIGH; x++) {
                sprintf(buf, GPIO_PARALLEL_PREFIX, x);
                #ifdef OS2
                rc = DosOpen(buf,&fh,&option,0,0,1,OPEN_FLAGS_FAIL_ON_ERROR|OPEN_SHARE_DENYREADWRITE,0);
                if(rc==0)
                {
                #endif

                fd = open (buf, O_RDONLY | O_NDELAY);
                if (fd != -1) {
                        close(fd);
                        list[*count].type = GPIO_DEVICE_PARALLEL;
                        strcpy(list[*count].path, buf);
                        sprintf(buf, "Parallel Port %i", x);
                        strcpy(list[*count].name, buf);
			list[*count].argument_needed = 0;
                        *count += 1;
                }
                #ifdef OS2
                }
                #endif
        }

        return (GPIO_OK);
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

int gpio_parallel_get_pin(gpio_device *dev, int pin) {

}

int gpio_parallel_set_pin(gpio_device *dev, int pin, int level) {

}

int gpio_parallel_update (gpio_device *dev) {

}
