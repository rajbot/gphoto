/* Serial port specific device functions. Do not use directly */
int 		gpio_serial_open(gpio_device *dev);
int 		gpio_serial_close(gpio_device *dev);

int 		gpio_serial_read(gpio_device *dev, char *bytes, int size);
int 		gpio_serial_write(gpio_device *dev, char *bytes, int size);

int 		gpio_serial_update (gpio_device *dev);

int 		gpio_serial_set_baudrate(gpio_device *dev);
static speed_t 	gpio_serial_baudconv(int rate);
