#include <stdio.h>
#include "io.h"

void dump (io_device *dev) {

	printf("port: %s\n", dev->port);
	printf("speed: %i\n", dev->speed);
}

int main(int argc, char **argv)
{
	io_device *dev;			/* declare the device */
	char buf[32];

	dev = io_new(IO_DEVICE_SERIAL);
					/* create a new serial device */
	strcpy(dev->port, "/dev/modem");
	dev->speed = 9600;
	io_open(dev);			/* open the device */
	dump(dev);

	dev->speed = 19200;
	io_reset(dev);			/* Apply the new speed */
	dump(dev);

	io_write(dev, "AT\n", 3);	/* write bytes to the device */
	
	io_read(dev, buf, 3);		/* read bytes from the device */
	buf[3] = 0;
	printf("recv: %s\n", buf);

	io_close(dev);			/* close the device */

	io_free(dev);
	return 0;
}
