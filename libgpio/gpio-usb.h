#ifndef GPIO_USB_H
#define GPIO_USB_H

/* USB port specific settings */
typedef struct {
	struct usb_device *udev;
	int  inep;
	int  outep;
	int  config;
	int  interface;
	int  altsetting;
} gpio_usb_settings;

extern struct gpio_operations gpio_usb_operations;

#endif /* GPIO_USB_H */
