/* Parallel port specific settings */
typedef struct {
        char port[128];
} gpio_parallel_settings;

extern struct gpio_operations gpio_parallel_operations;
