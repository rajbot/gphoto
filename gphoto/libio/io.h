#ifndef IO_H
#define IO_H

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>

#ifndef TRUE
#define TRUE (0==0)
#endif

#ifndef FALSE
#define FALSE (1==0)
#endif

#if HAVE_TERMIOS_H
# include <termios.h>
#else
#  if HAVE_SYS_IOCTL_H
#   include <sys/ioctl.h>
#  endif
#  include <sgtty.h>
# endif

#define IO_MAX_BUF_LEN 4096 		/* max length of receive buffer */


#define	IO_ERROR	-2		/* IO return codes */
#define IO_OK		-1

/* Specify the types of devices */
typedef enum {
	IO_DEVICE_SERIAL,
	IO_DEVICE_USB,
	IO_DEVICE_1394,
	IO_DEVICE_PARALLEL
} io_device_type;

/* Specify the device information */
typedef struct
{
        io_device_type type; 

	/*
	   What follows are the device specific options.
	   They are listed in here instead of a union so that
	   adjusting settings are straight forward. See the 
	   test.c for examples. There may be a better way to
	   do it, so please drop some mail. -Scott
	 */

	/* Serial port options start */
        char port[128];
       	int  speed;
        int  bits;
        int  parity;
        int  stopbits;
	/* Serial port options end */

	/* USB options start */
	int  bus;
	int  device;
	/* USB options end */

	/* IEEE1394 options start */
	/* IEEE1394 options end */

	/* Parallel port options start */
	/* Parallel port options end */

	/* do not access the following: */
	int device_fd;
} io_device;


io_device *io_new		(io_device_type type);
		/* Create a new device of type "type"
			return values:
				  successful: valid io_device struct
				unsuccessful: IO_ERROR
		*/

	int io_open       	(io_device *dev);
		/* Open the device for reading and writing
			return values:
				  successful: IO_OK
				unsuccessful: IO_ERROR
		*/

	int io_close      	(io_device *dev);
		/* Close the device to prevent reading and writing
			return values:
				  successful: IO_OK
				unsuccessful: IO_ERROR
		*/



       int io_reset 		(io_device *dev);
		/* Used to apply any changed configuration settings
		   Call this if you set/change the baud rate, for example
			return values:
				  successful: IO_OK
				unsuccessful: IO_ERROR
		 */

	int io_free      	(io_device *dev);
		/* Frees an IO device from memory
			return values:
				  successful: IO_OK
				unsuccessful: IO_ERROR
		*/

       int io_write 		(io_device *dev, char *bytes, int size);
		/* Writes "bytes" of size "size" to the device
			return values:
				  successful: IO_OK
				unsuccessful: IO_ERROR
		*/

       int io_read		(io_device *dev, char *bytes, int size);
		/* Reads "size" bytes in to "bytes" from the device
			return values:
				  successful: number of bytes read
				unsuccessful: IO_ERROR
		*/

#endif

