/* PARALLEL port prefix for enumeration */

/* Linux */
#ifdef __linux__
#define GPIO_PARALLEL_PREFIX "/dev/lp%i"
#endif

/* BSD */
#if defined(__FreeBSD__) || defined(__NetBSD__)
#define GPIO_PARALLEL_PREFIX NULL
#endif

/* Solaris */
#ifdef sun
#define GPIO_PARALLEL_PREFIX NULL
#endif

/* BeOS */
#ifdef beos
/* ????????????? */
#define GPIO_PARALLEL_PREFIX NULL
#endif

/* Windows */
#ifdef WIN
#define GPIO_PARALLEL_PREFIX "LPT%i:"
#endif

/* Others? */

/* Default */
#ifndef GPIO_PARALLEL_PREFIX
#define GPIO_PARALLEL_PREFIX NULL
#endif

/* PARALLEL port specific settings */
typedef struct {
        char port[128];
} gpio_parallel_settings;

extern struct gpio_operations gpio_parallel_operations;
