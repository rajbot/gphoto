#ifdef WIN32
#define DLOPEN(_handle, _filename)
#define DLSYM(_pointer, _handle, _funcname)
#define DLCLOSE(_handle)
#define DLERROR()
#else
#include <dlfcn.h>
#define DLOPEN(_handle, _filename)		(_handle = dlopen(_filename, RTLD_LAZY))
#define DLSYM(_pointer, _handle, _funcname)	(_pointer = dlsym(_handle, _funcname))
#define DLCLOSE(_handle)			(dlclose(_handle))
#define DLERROR()				(dlerror())
#endif

int gpio_library_list(gpio_device_info *list, int *count);
int gpio_library_load(gpio_device *device, gpio_device_type type);
int gpio_library_close(gpio_device *device);
