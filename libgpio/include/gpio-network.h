#ifndef _GPIO_NETWORK_H_
#define _GPIO_NETWORK_H_

/* socket specific settings */
typedef struct {
	char address[20];
} gpio_network_settings;

extern struct gpio_operations gpio_network_operations;

#endif /* _GPIO_NETWORK_H_ */




