/* Windows GPIO serial library.
	Scott Fritzinger <scottf@unr.edu>
	Covered by the LGPL
 */

#include <string.h>
#include <stdio.h>
#include "gpio.h"

/* Serial prototypes
   ------------------------------------------------------------------ */
int 		gpio_serial_init(gpio_device *dev);
int 		gpio_serial_exit(gpio_device *dev);

int 		gpio_serial_open(gpio_device *dev);
int 		gpio_serial_close(gpio_device *dev);

int 		gpio_serial_read(gpio_device *dev, char *bytes, int size);
int 		gpio_serial_write(gpio_device *dev, char *bytes, int size);

int			gpio_serial_get_pin(gpio_device *dev, int pin);
int     	gpio_serial_set_pin(gpio_device *dev, int pin, int level); 

int 		gpio_serial_update (gpio_device *dev);

/* Dynamic library functions
   --------------------------------------------------------------------- */

gpio_device_type gpio_library_type () {

        return (GPIO_DEVICE_SERIAL);
}

gpio_operations *gpio_library_operations () {

        gpio_operations *ops;

        ops = (gpio_operations*)malloc(sizeof(gpio_operations));
		memset(ops, 0, sizeof(gpio_operations));

        ops->init   = gpio_serial_init;
        ops->exit   = gpio_serial_exit;
        ops->open   = gpio_serial_open;
        ops->close  = gpio_serial_close;
        ops->read   = gpio_serial_read;
        ops->write  = gpio_serial_write;
        ops->update = gpio_serial_update;
        ops->get_pin = gpio_serial_get_pin;
        ops->set_pin = gpio_serial_set_pin;

        return (ops);
}

int gpio_library_list (gpio_device_info *list, int *count) {

	
	char buf[1024], prefix[1024];
	int x;
	HANDLE hComm;

	strcpy(prefix, GPIO_SERIAL_PREFIX);

	for (x=GPIO_SERIAL_RANGE_LOW; x<=GPIO_SERIAL_RANGE_HIGH; x++) {
		sprintf(buf, prefix, x);
		hComm = CreateFile(buf, GENERIC_READ | GENERIC_WRITE,0,0,OPEN_EXISTING,0,0);
		if (hComm != INVALID_HANDLE_VALUE) {
			CloseHandle(hComm);
			list[*count].type = GPIO_DEVICE_SERIAL;
			strcpy(list[*count].path, buf);
			sprintf(buf, "Serial Port %i", x);
			strcpy(list[*count].name, buf);
			list[*count].argument_needed = 0;
			*count += 1;
		}
	}

	return (GPIO_OK);
}


/* Serial API functions
   ------------------------------------------------------------------ */

int gpio_serial_init (gpio_device *dev) {
	/* save previous setttings in to dev->settings_saved */

//printf("init\n");

	dev->device_handle = INVALID_HANDLE_VALUE;
	
	/* Copy in some 'safe' default values */
	strcpy(dev->settings.serial.port, "COM1:");
	dev->settings.serial.speed = 9600;
	dev->settings.serial.bits = 8;
	dev->settings.serial.parity = 0;
	dev->settings.serial.stopbits = 1;

	return GPIO_OK;
}

int gpio_serial_exit (gpio_device *dev) {
	/* do anything required when completely done with the port */
//printf("exit\n");

	return GPIO_OK;
}

int gpio_serial_open(gpio_device * dev)
{

	DCB dcb;
	char buf[1024];

//	printf("open\n");
	FillMemory(&dcb, sizeof(dcb), 0);
	dcb.DCBlength = sizeof(dcb);
	sprintf(buf, "%i,%c,%i,%i",
		dev->settings.serial.speed,
		dev->settings.serial.parity == 1? 'y':'n',
		dev->settings.serial.bits,
		dev->settings.serial.stopbits);
	if (!BuildCommDCB(buf, &dcb)) {
		printf("BuildCommDCB\n");
		return GPIO_ERROR;
	}
//printf("port=%s\n", dev->settings.serial.port);
	dev->device_handle = CreateFile(dev->settings.serial.port,
		GENERIC_READ | GENERIC_WRITE,0,0,OPEN_EXISTING,0,0);
	if (dev->device_handle == INVALID_HANDLE_VALUE) {
		printf("CreateFile\n");
		return (GPIO_ERROR);
	}

	if (!SetCommState(dev->device_handle, &dcb)) {
		CloseHandle(dev->device_handle);
		printf("SetCommState\n");
		return (GPIO_ERROR);
	}

	return GPIO_OK;
}

int gpio_serial_close(gpio_device * dev)
{
//	printf("close\n");
	CloseHandle(dev->device_handle);
	return GPIO_OK;
}

int gpio_serial_write(gpio_device * dev, char *bytes, int size)
{
	int len;
	COMMTIMEOUTS t;

//	printf("write\n");
	len = 0;
	t.ReadIntervalTimeout		 = dev->timeout; 
    t.ReadTotalTimeoutMultiplier = 1; 
    t.ReadTotalTimeoutConstant   = 100000; 
	t.WriteTotalTimeoutMultiplier= 1; 
    t.WriteTotalTimeoutConstant  = 100000; 
	if (!SetCommTimeouts(dev->device_handle, &t)) {
		printf("SetCommTimeouts\n");
		return GPIO_ERROR;
	}

	WriteFile(dev->device_handle, bytes, size, &len, NULL); 
	if (size != len)
		printf("Timeout!*******************************\n");
	
	return GPIO_OK;
}


int gpio_serial_read(gpio_device * dev, char *bytes, int size)
{
	int len = 0;
	COMMTIMEOUTS t;

//	printf("read\n");
	t.ReadIntervalTimeout		 = dev->timeout; 
    t.ReadTotalTimeoutMultiplier = 10;
    t.ReadTotalTimeoutConstant   = dev->timeout;
	t.WriteTotalTimeoutMultiplier= 10; 
    t.WriteTotalTimeoutConstant  = dev->timeout; 
	if (!SetCommTimeouts(dev->device_handle, &t)) {
		printf("SetCommTimeouts\n");
		return GPIO_ERROR;
	}

	ReadFile(dev->device_handle, bytes, size, &len, NULL);
	if (len != size)
		/* Better way to check? */
		return GPIO_TIMEOUT;

	return size;
}

/*
 * Get the status of the lines of the serial port
 * 
 */
int gpio_serial_get_pin(gpio_device * dev, int pin)
{
//	int bit;
//	printf("get_pin\n");
	switch(pin) {
		case PIN_RTS:

			break;
		case PIN_DTR:
			
			break;
		case PIN_CTS:

			break;
		case PIN_DSR:

			break;
		case PIN_CD:

			break;
		case PIN_RING:

			break;
		default:
			return GPIO_ERROR;
	}

	return (0);
}

/*
* Set the status of lines in the serial port
*
* level is 0 for off and 1 for on
*
*/
int gpio_serial_set_pin(gpio_device * dev, int pin, int level)
{
//	int bit;
//	printf("set_pin\n");
	switch(pin) {
		case PIN_RTS:

			break;
		case PIN_DTR:

			break;
		case PIN_CTS:

			break;
		case PIN_DSR:

			break;
		case PIN_CD:

			break;
		case PIN_RING:

			break;
		default:
			return GPIO_ERROR;
	}

	return GPIO_OK;	
}

/*
 * This function will apply the settings to
 * the device. The device has to be opened
 */
int gpio_serial_update(gpio_device * dev)
{
	DCB dcb;
	char buf[1024];
//	printf("update\n");
	memcpy(&dev->settings, &dev->settings_pending, sizeof(dev->settings));

	if (dev->device_handle == INVALID_HANDLE_VALUE)
		/* Device hasn't been opened yet */
		return GPIO_OK;

	sprintf(buf, "%i,%c,%i,%i",
		dev->settings.serial.speed,
		dev->settings.serial.parity == 1? 'y':'n',
		dev->settings.serial.bits,
		dev->settings.serial.stopbits);
	FillMemory(&dcb, sizeof(dcb), 0);
	dcb.DCBlength = sizeof(dcb);
	if (!BuildCommDCB(buf, &dcb)) {
		printf("BuildCommDCB\n");
		return GPIO_ERROR;
	}
	if (!SetCommState(dev->device_handle, &dcb)) {
		printf("SetCommState\n");
		return (GPIO_ERROR);
	}
	return GPIO_OK;
}

