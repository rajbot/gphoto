/* Serial port prefix for enumeration */
/* %i for numbers, %c for letters */

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
