#ifndef _GPIO_SOCKET_H_
#define _GPIO_SOCKET_H_

#define GPIO_SOCKET

/* socket specific settings */
typedef struct {
        char fqdn[128];
	char ip[20];
} gpio_socket_settings;

extern struct gpio_operations gpio_socket_operations;

#endif /* _GPIO_SOCKET_H_ */







