#ifndef _GPIO_PARALLEL_H_
#define _GPIO_PARALLEL_H_

#define GPIO_PARALLEL

/* PARALLEL port prefix for enumeration */

/* Linux */
#ifdef linux
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
#define GPIO_PARALLEL_PREFIX 		NULL
#define	GPIO_PARALLEL_RANGE_LOW		0
#define	GPIO_PARALLEL_RANGE_HIGH	16
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




