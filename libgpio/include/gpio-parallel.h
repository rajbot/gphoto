#ifndef _GPIO_PARALLEL_H_
#define _GPIO_PARALLEL_H_

/* PARALLEL port prefix for enumeration */

/* Linux */
#ifdef __linux
#define GPIO_PARALLEL_PREFIX 		"/dev/lp%i"
#define	GPIO_PARALLEL_RANGE_LOW		0
#define	GPIO_PARALLEL_RANGE_HIGH	16
#endif

/* BSD */
#if defined(__FreeBSD__) || defined(__NetBSD__)
#define GPIO_PARALLEL_PREFIX 		NULL
#define	GPIO_PARALLEL_RANGE_LOW		0
#define	GPIO_PARALLEL_RANGE_HIGH	0
#endif

/* Solaris */
#ifdef sun
#  ifdef i386 
#    define GPIO_PARALLEL_PREFIX "/dev/lp%i" /* x86 parallel port prefix*/
#    define GPIO_PARALLEL_RANGE_LOW	1
#    define GPIO_PARALLEL_RANGE_HIGH	16
#  else
#    define GPIO_PARALLEL_PREFIX "/dev/bpp%02i" /* Sparc parallel port prefix*/
#    define GPIO_PARALLEL_RANGE_LOW	0
#    define GPIO_PARALLEL_RANGE_HIGH	16
#  endif
#endif


/* BeOS */
#ifdef beos
/* ????????????? */
#define GPIO_PARALLEL_PREFIX		NULL
#define	GPIO_PARALLEL_RANGE_LOW		0
#define	GPIO_PARALLEL_RANGE_HIGH	0
#endif

/* Windows */
#ifdef WIN
#define GPIO_PARALLEL_PREFIX 		"LPT%i:"
#define	GPIO_PARALLEL_RANGE_LOW		0
#define	GPIO_PARALLEL_RANGE_HIGH	16
#endif

/* OS/2 */
#ifdef OS2
#define GPIO_PARALLEL_PREFIX "LPT%i"
#define GPIO_PARALLEL_RANGE_LOW         1
#define GPIO_PARALLEL_RANGE_HIGH        4
#endif

/* Others? */

/* Default */
#ifndef GPIO_PARALLEL_PREFIX
#warning GPIO_PARALLEL_PREFIX not defined. Enumeration will fail
#define GPIO_PARALLEL_PREFIX 		NULL
#define	GPIO_PARALLEL_RANGE_LOW		0
#define	GPIO_PARALLEL_RANGE_HIGH	0
#endif

/* PARALLEL port specific settings */
typedef struct {
        char port[128];
} gpio_parallel_settings;

extern struct gpio_operations gpio_parallel_operations;

#endif /* _GPIO_PARALLEL_H_ */


