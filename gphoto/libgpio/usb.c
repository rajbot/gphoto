/*
	The (gPhoto) I/O Library (USB)

	Written by:
	Copyright 1999-2000, Johannes Erdfelt <johannes@erdfelt.com>

	This library is covered by the LGPL.
*/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h> 
#include <fcntl.h> 
#include <errno.h> 
#include <sys/time.h>
#include <dirent.h>

#include <usb.h>

#include "gpio.h"

int gpio_usb_init(gpio_device *dev)
{
	usb_init();
}

int gpio_usb_open(gpio_device *dev)
{
	int ret;

	dev->device_handle = usb_open(dev->settings.usb.udev);
	if (!dev->device_handle)
		return GPIO_ERROR;

	ret = usb_set_configuration(dev->device_handle, dev->settings.usb.config);
	if (ret < 0) {
		fprintf(stderr, "gpio_usb_open: could not set config %d: %s\n",
			dev->settings.usb.config, strerror(errno));
		return GPIO_ERROR;
	}

	ret = usb_claim_interface(dev->device_handle, dev->settings.usb.interface);
	if (ret < 0) {
		fprintf(stderr, "gpio_usb_open: could not claim intf %d: %s\n",
			dev->settings.usb.interface, strerror(errno));
		return GPIO_ERROR;
	}

	ret = usb_set_altinterface(dev->device_handle, dev->settings.usb.altsetting);
	if (ret < 0) {
		fprintf(stderr, "gpio_usb_open: could not set intf %d/%d: %s\n",
			dev->settings.usb.interface,
			dev->settings.usb.altsetting, strerror(errno));
		return GPIO_ERROR;
	}

	return GPIO_OK;
}

int gpio_usb_close(gpio_device *dev)
{
	if (usb_close(dev->device_handle) < 0)
		fprintf(stderr, "gpio_usb_close: %s\n",
			usb_strerror());

	dev->device_handle = NULL;

	return GPIO_OK;
}

int gpio_usb_reset(gpio_device *dev)
{
	gpio_usb_close(dev);
	gpio_usb_open(dev);

	return GPIO_OK;
}

int gpio_usb_write(gpio_device *dev, char *bytes, int size)
{
	return usb_bulk_write(dev->device_handle, dev->settings.usb.outep,
		bytes, size, dev->timeout);
}

int gpio_usb_read(gpio_device *dev, char *bytes, int size)
{
	return usb_bulk_read(dev->device_handle, dev->settings.usb.inep,
		bytes, size, dev->timeout);
}


/*
 * This function applys changes to the device
 * (At this time it does nothing)
 */
int gpio_usb_update(gpio_device *dev)
{
	memcpy(&dev->settings, &dev->settings_pending, sizeof(dev->settings));
	if (dev->device_handle) {
		if (gpio_usb_close(dev) == GPIO_ERROR)
			return GPIO_ERROR;
		if (gpio_usb_open(dev) == GPIO_ERROR)
			return GPIO_ERROR;
	}

	return GPIO_OK;
}

int gpio_usb_msg_write(gpio_device * dev, int value, char *bytes, int size)
{
        return usb_control_msg(dev->device_handle,
                USB_TYPE_VENDOR | USB_RECIP_DEVICE,
                size > 1 ? 0x04 : 0x0c, value, 0, bytes, size, dev->timeout);
}


int gpio_usb_msg_read(gpio_device * dev, int value, char *bytes, int size)
{
        return usb_control_msg(dev->device_handle,
                USB_TYPE_VENDOR | USB_RECIP_DEVICE | 0x80,
                size > 1 ? 0x04 : 0x0c, value, 0, bytes, size, dev->timeout);
}

int gpio_usb_clear_halt(gpio_device * dev)
{
        if (usb_clear_halt(dev->device_handle, dev->settings.usb.inep)
                || usb_clear_halt(dev->device_handle, dev->settings.usb.outep))
                return GPIO_ERROR;
        else
                return GPIO_OK;
}

int gpio_usb_find_device(int idvendor, int idproduct, struct usb_device **device)
{
  struct usb_bus *bus;
  struct usb_device *dev;

  for (bus = usb_busses; bus; bus = bus->next) {
    for (dev = bus->devices; dev; dev = dev->next) {

      if ((dev->descriptor.idVendor == idvendor) &&
          (dev->descriptor.idProduct == idproduct)) {
        *device = dev;
        return 1;
      }
    }
  }

  return 0;
}

int gpio_usb_get_pin(gpio_device * dev, int pin) {
	/* non sensical */

	return (GPIO_ERROR);
}

int gpio_usb_set_pin(gpio_device * dev, int pin, int level) {
	/* non sensical */

	return (GPIO_ERROR);
}

struct gpio_operations gpio_usb_operations = {
	gpio_usb_init,
	gpio_usb_open,
	gpio_usb_close,
	gpio_usb_read,
	gpio_usb_write,
	gpio_usb_update,
	gpio_usb_get_pin,
	gpio_usb_set_pin
};
