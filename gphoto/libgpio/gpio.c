/*
	The (gPhoto) I/O Library

	Modified by:
	Copyright 1999, Scott Fritzinger <scottf@unr.edu>

	Based on work by:
 	Copyright 1999, Beat Christen <spiff@longstreet.ch>
		for the toshiba gPhoto library.

	This library is covered by the LGPL.
*/

#include "gpio.h"

/*
   Core IO library functions
   ----------------------------------------------------------------
*/

void gpio_init(void)
{
#ifdef GPIO_USB
	usb_init();
	usb_find_busses();
	usb_find_devices();
#endif
}

gpio_device *gpio_new(gpio_device_type type)
	/* Create a new IO device */
{
	gpio_device *dev;

	dev = (gpio_device *)malloc (sizeof(gpio_device));
	dev->type = type;
	dev->device_fd=0;

	switch (type) {
        case GPIO_DEVICE_SERIAL:
		dev->ops = &gpio_serial_operations;
		dev->timeout = 10 * 1000;
		break;
#ifdef GPIO_USB
        case GPIO_DEVICE_USB:
		dev->ops = &gpio_usb_operations;
		dev->timeout = 5 * 1000;
		break;
#endif
        case GPIO_DEVICE_1394:
		break;
        case GPIO_DEVICE_PARALLEL:
		break;
	}

	dev->ops->init(dev);

	return (dev);
}

int gpio_open(gpio_device *dev)
	/* Open a device for reading/writing */
{
	int retval = 0;
	
	/* Try to open device */
	retval = dev->ops->open(dev);
	if (retval == GPIO_OK) {
		/* Now update the settings */
		retval = dev->ops->update(dev);
		if (retval != GPIO_OK) {
			dev->device_fd = 0;
			return GPIO_ERROR;
		}
		return GPIO_OK;
	}

	return GPIO_ERROR;
}

int gpio_close(gpio_device *dev)
	/* Close the device to prevent reading/writing */
{
	int retval=0;
	
	if (!dev)
		return GPIO_ERROR;
	if (dev->device_fd==0)
		return GPIO_OK;

	retval=dev->ops->close(dev);
	dev->device_fd=0;
	
	return retval;
}

int gpio_free(gpio_device *dev)
	/* Frees a device struct */
{
	free(dev);

	return GPIO_OK;
}

int gpio_write(gpio_device *dev, char *bytes, int size)
	/* Called to write "bytes" to the IO device */
{
	return dev->ops->write(dev, bytes, size);
}

int gpio_read(gpio_device *dev, char *bytes, int size) 
	/* Reads data from the device into the "bytes" buffer.
	   "bytes" should be large enough to hold all the data.
	*/
{
	return dev->ops->read(dev, bytes, size);
}

int gpio_set_timeout(gpio_device *dev, int millisec_timeout)
{
	dev->timeout = millisec_timeout;
	return GPIO_OK;
}

int gpio_get_timeout(gpio_device *dev, int *millisec_timeout)
{
	*millisec_timeout = dev->timeout;
	return GPIO_OK;
}

int gpio_set_settings(gpio_device *dev, gpio_device_settings settings)
{
	/* need to memcpy() settings to dev->settings */
	memcpy(&dev->settings_pending,&settings,sizeof(dev->settings_pending));

	return dev->ops->update (dev);	
}


int gpio_get_settings(gpio_device *dev, gpio_device_settings *settings)
{
	memcpy(settings, &dev->settings, sizeof(gpio_device_settings));	
	return GPIO_OK;
}


int gpio_get_pin   (gpio_device *dev, int pin) {

	return (dev->ops->get_pin (dev, pin));
}

int gpio_set_pin   (gpio_device *dev, int pin, int level) {

	return (dev->ops->set_pin (dev, pin, level));
}
