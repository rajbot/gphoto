#ifndef _GPIO_SERIAL_H_
#define _GPIO_SERIAL_H_

/* Serial port prefix for enumeration */
/* %i for numbers, %c for letters */
/* also define the low and high values of the range to check for devices */


/* Linux */
#ifdef __linux
/* devfs is accounted for in the implementation */
#define GPIO_SERIAL_PREFIX 	"/dev/ttyS%i"
#define GPIO_SERIAL_RANGE_LOW	0
#define GPIO_SERIAL_RANGE_HIGH	32
#endif

/* BSD */
#if defined(__FreeBSD__) || defined(__NetBSD__)
#define GPIO_SERIAL_PREFIX	"/dev/tty0%i"
#define GPIO_SERIAL_RANGE_LOW	0
#define GPIO_SERIAL_RANGE_HIGH	32
#endif

/* Solaris */
#ifdef sun
#define GPIO_SERIAL_PREFIX "/dev/tty%c"
#define GPIO_SERIAL_RANGE_LOW	'a'
#define GPIO_SERIAL_RANGE_HIGH	'z'
#endif

/* BeOS */
#ifdef beos
/* ????????????? */
#define GPIO_SERIAL_PREFIX NULL
#define GPIO_SERIAL_RANGE_LOW	0
#define GPIO_SERIAL_RANGE_HIGH	0
#endif

/* Windows */
#ifdef WIN32
#define GPIO_SERIAL_PREFIX 	"COM%i:"
#define GPIO_SERIAL_RANGE_LOW	1
#define GPIO_SERIAL_RANGE_HIGH	4
#endif

#ifdef OS2
#define GPIO_SERIAL_PREFIX 	"COM%i"
#define GPIO_SERIAL_RANGE_LOW   1
#define GPIO_SERIAL_RANGE_HIGH  4
#endif

/* Others? */

/* Default */
#ifndef GPIO_SERIAL_PREFIX
#define GPIO_SERIAL_PREFIX 	"/dev/cua%i"
#define GPIO_SERIAL_RANGE_LOW	0
#define GPIO_SERIAL_RANGE_HIGH	0
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
