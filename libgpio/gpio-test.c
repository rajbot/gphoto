
#include <stdio.h>
#include <string.h>
#include "gpio.h"

void dump(gpio_device * dev)
{

}

int main(int argc, char **argv)
{
	gpio_device *dev;	/* declare the device */
	gpio_device_settings settings;
	char buf[32];

	dev = gpio_new(0);
	/* create a new serial device */
	gpio_set_timeout(dev, 500);

	strcpy(settings.serial.port, "/dev/modem");
	settings.serial.speed = 19200;
	settings.serial.bits = 8;
	settings.serial.parity = 0;
	settings.serial.stopbits = 1;

	gpio_set_settings(dev, settings);
	gpio_open(dev);		/* open the device */
	dump(dev);

	gpio_get_settings(dev, &settings);
	settings.serial.speed = 57600;
	gpio_set_settings(dev, settings);

	dump(dev);

	printf("CTS: %i", gpio_get_pin(dev,PIN_CTS));

	gpio_write(dev, "AT\n", 3);	/* write bytes to the device */

	gpio_read(dev, buf, 3);	/* read bytes from the device */
	buf[3] = 0;
	printf("recv: %s\n", buf);

	gpio_close(dev);	/* close the device */

	gpio_free(dev);

	return 0;
}
