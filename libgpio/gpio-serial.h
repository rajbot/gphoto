/* Serial port specific settings */
typedef struct {
        char port[128];
        int  speed;
        int  bits;
        int  parity;
        int  stopbits;
} gpio_serial_settings;

extern struct gpio_operations gpio_serial_operations;   
