#ifndef _GPIO_SERIAL_H_
#define _GPIO_SERIAL_H_

#define GPIO_SERIAL

/* Serial port prefix for enumeration */
/* %i for numbers, %c for letters */
/* also define the low and high values of the range to check for devices */

#define GPIO_SERIAL_RANGE_LOW	0
#define GPIO_SERIAL_RANGE_HIGH	32

/* Linux */
#ifdef linux
#define GPIO_SERIAL_PREFIX "/dev/ttyS%i"
#endif

/* BSD */
#if defined(__FreeBSD__) || defined(__NetBSD__)
#define GPIO_SERIAL_PREFIX "/dev/tty0%i"
#endif

/* Solaris */
#ifdef sun
#define GPIO_SERIAL_PREFIX "/dev/ttya%i"
#endif

/* BeOS */
#ifdef beos
/* ????????????? */
#define GPIO_SERIAL_PREFIX NULL
#endif

/* Windows */
#ifdef WIN
#define GPIO_SERIAL_PREFIX "COM%i:"
#endif

/* Others? */

/* Default */
#ifndef GPIO_SERIAL_PREFIX
#define GPIO_SERIAL_PREFIX NULL
#endif

/* Serial port specific settings */
typedef struct {
        char port[128];
        int  speed;
        int  bits;
        int  parity;
        int  stopbits;
} gpio_serial_settings;

extern struct gpio_operations gpio_serial_operations;   

#define PIN_RTS 0
#define PIN_DTR 1
#define PIN_CTS 2
#define PIN_DSR 3
#define PIN_CD  4
#define PIN_RING 5 



#endif /* _GPIO_SERIAL_H_ */




