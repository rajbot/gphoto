#ifndef _GPIO_H_
#define _GPIO_H_

#ifdef OS2
#include <gpioos2.h>
#include <os2.h>
#endif

/* Include the portability layer */
#include "gpio-port.h"

/* Include serial by default */
#include "gpio-serial.h"

#ifdef GPIO_PARALLEL
#include "gpio-parallel.h"
#endif

#ifdef GPIO_NETWORK
#include "gpio-network.h"
#endif

#ifdef GPIO_USB
#include <usb.h>
#include "gpio-usb.h"
#endif

#ifdef GPIO_IEEE1394
#include "gpio-ieee1394.h"
#endif


#ifndef TRUE
#define TRUE (0==0)
#endif

#ifndef FALSE
#define FALSE (1==0)
#endif

/* Defines and enums
   -------------------------------------------------------------- */

#define GPIO_MAX_BUF_LEN 4096 		/* max length of receive buffer */


/* Return values (errors are negative) */
#define GPIO_OK		0
#define	GPIO_ERROR	-1		/* IO return codes */
#define GPIO_TIMEOUT    -2

/* Debugging definitions for gpio_init */
#define GPIO_DEBUG_NONE		0
#define GPIO_DEBUG_LOW		1
#define GPIO_DEBUG_MEDIUM 	2
#define GPIO_DEBUG_HIGH 	3

/* Specify the types of devices */
typedef enum {
	GPIO_DEVICE_SERIAL,
	GPIO_DEVICE_PARALLEL,	/* <- Not supported yet */
	GPIO_DEVICE_NETWORK,	/* <- Not supported yet */
	GPIO_DEVICE_USB,
	GPIO_DEVICE_IEEE1394, 	/* <- Not supported yet */
} gpio_device_type;



/* Device struct
   -------------------------------------------------------------- */
typedef struct {
	gpio_device_type type;
	char name[64];
	char path[64];
	
	/* not used yet */
	int  argument_needed;
	char argument_description[128];
	char argument[128];

	/* don't touch */
	char library_filename[1024];
} gpio_device_info;

/* Put the settings together in a union */
typedef union {
	gpio_serial_settings	serial;
#ifdef GPIO_PARALLEL
	gpio_parallel_settings	parallel;
#endif
#ifdef GPIO_NETWORK
	gpio_network_settings	network;
#endif
#ifdef GPIO_USB
	gpio_usb_settings 	usb;
#endif
#ifdef GPIO_IEEE1394
	gpio_ieee1394_settings 	ieee1394;
#endif

} gpio_device_settings;

#ifdef GPIO_USB
enum {
        GPIO_USB_IN_ENDPOINT,
	GPIO_USB_OUT_ENDPOINT
};
#endif


struct gpio_device;
typedef struct gpio_device gpio_device;
struct gpio_operations {
	int (*init)	(gpio_device *);
	int (*exit)	(gpio_device *);
	int (*open)	(gpio_device *);
	int (*close)	(gpio_device *);
	int (*read)	(gpio_device *, char *, int);
	int (*write)	(gpio_device *, char *, int);
	int (*update)   (gpio_device *);

	/* for serial and parallel devices */
	int (*get_pin)	 (gpio_device *, int);
	int (*set_pin)	 (gpio_device *, int, int);
	int (*send_break)(gpio_device *, int);

#ifdef GPIO_USB
	/* for USB devices */
	int (*find_device)(gpio_device * dev, int idvendor, int idproduct);
	int (*clear_halt) (gpio_device * dev, int ep);
	int (*msg_write)  (gpio_device * dev, int value, char *bytes, int size);
	int (*msg_read)   (gpio_device * dev, int value, char *bytes, int size);
#endif
};

typedef struct gpio_operations gpio_operations;

/* Function pointers for the dynamic libraries */
typedef int		 (*gpio_ptr_type) 	();
typedef int 		 (*gpio_ptr_list) 	(gpio_device_info*, int *);
typedef gpio_operations* (*gpio_ptr_operations) ();

/* Specify the device information */
struct gpio_device {
	/* This struct is available via wrappers. don't modify 
	   directly. */
        gpio_device_type type;

	gpio_operations *ops;

	gpio_device_settings settings;
	gpio_device_settings settings_pending;

	gpio_device_settings settings_saved;

	int device_fd;
#ifdef WIN32
	HANDLE device_handle;
#else
	void *device_handle;
#endif
	int timeout; /* in milli seconds */

	void *library_handle;

#ifdef GPIO_USB
	struct usb_device *usb_device;
#endif

	int debug_level;
};

/* gpio Core functions
   -------------------------------------------------------------- */

	void gpio_debug_printf (int target_debug_level, int debug_level, char *format, ...);
		/* issues debugging messages */

	int gpio_init (int debug_level);
		/* Initializes the library.
			return values:
				  successful: GPIO_OK
				unsuccessful: GPIO_ERROR
		*/

	int gpio_get_device_count ();
		/* Get a count of available devices
			return values:
				  successful: valid gpio_device_list struct
				unsuccessful: GPIO_ERROR
		*/

	int gpio_get_device_info (int device_number, gpio_device_info *info);
		/* Get information about a device
			return values:
				  successful: valid gpio_device_list struct
				unsuccessful: GPIO_ERROR
		*/

gpio_device *gpio_new		(gpio_device_type type);
		/* Create a new device of type "type"
			return values:
				  successful: valid gpio_device struct
				unsuccessful: GPIO_ERROR
		*/

	int gpio_free      	(gpio_device *dev);
		/* Frees an IO device from memory
			return values:
				  successful: GPIO_OK
				unsuccessful: GPIO_ERROR
		*/

	int gpio_set_debug (gpio_device *dev, int debug_level);
		/* 
			Set the debugging level specific to a device 
		*/

	int gpio_open       	(gpio_device *dev);
		/* Open the device for reading and writing
			return values:
				  successful: GPIO_OK
				unsuccessful: GPIO_ERROR
		*/

	int gpio_close      	(gpio_device *dev);
		/* Close the device to prevent reading and writing
			return values:
				  successful: GPIO_OK
				unsuccessful: GPIO_ERROR
		*/

       int gpio_set_timeout 	(gpio_device *dev, int millisec_timeout);
		/* Sets the read/write timeout
				  successful: GPIO_OK
				unsuccessful: GPIO_ERROR
		*/

       int gpio_get_timeout 	(gpio_device *dev, int *millisec_timeout);
		/* Sets the read/write timeout
				  successful: GPIO_OK
				unsuccessful: GPIO_ERROR
		*/

       int gpio_set_settings 	(gpio_device *dev, 
				 gpio_device_settings settings);
		/* Sets the settings
				  successful: GPIO_OK
				unsuccessful: GPIO_ERRO
		*/


       int gpio_get_settings 	(gpio_device *dev, 
				 gpio_device_settings *settings);
		/* Returns settings in "settings"
				  successful: GPIO_OK
				unsuccessful: GPIO_ERROR
		*/

       int gpio_write 		(gpio_device *dev, char *bytes, int size);
		/* Writes "bytes" of size "size" to the device
			return values:
				  successful: GPIO_OK
				unsuccessful: GPIO_ERROR
		*/

       int gpio_read		(gpio_device *dev, char *bytes, int size);
		/* Reads "size" bytes in to "bytes" from the device
			return values:
				  successful: number of bytes read
				unsuccessful: GPIO_ERROR
		*/


/* Serial and Parallel specific functions
   -------------------------------------------------------------- */

	int gpio_get_pin   (gpio_device *dev, int pin);
		/* Give the status of pin from dev
			pin values:
				 see PIN_ constants in the various .h files	
			return values:
				  successful: status
				unsuccessful: GPIO_ERROR
		*/

	int gpio_set_pin   (gpio_device *dev, int pin, int level);
		/* set the status of pin from dev to level
			pin values:
				 see PIN_ constants in the various .h files	
			level values:
					0 for off
					1 for on
			return values:
				  successful: status
				unsuccessful: GPIO_ERROR
		*/

	int gpio_send_break (gpio_device *dev, int duration);
		/* send a break (duration is in seconds) */

/* USB specific functions
   -------------------------------------------------------------- */
#ifdef GPIO_USB
	/* must port libusb to other platforms for this to drop-in */
	int gpio_usb_find_device (gpio_device * dev, int idvendor, int idproduct);
	int gpio_usb_clear_halt  (gpio_device * dev, int ep);
	int gpio_usb_msg_write   (gpio_device * dev, int value, char *bytes, int size);
	int gpio_usb_msg_read    (gpio_device * dev, int value, char *bytes, int size);
#endif

#endif /* _GPIO_H_ */
