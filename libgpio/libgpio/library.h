#ifdef WIN32
#include <windows.h>
#define GP_DLOPEN(_handle, _filename)			(_handle = LoadLibrary(_filename))
#define GP_DLSYM(_pointer, _handle, _funcname)	(_pointer = GetProcAddress(_handle, _funcname))
#define GP_DLCLOSE(_handle)						(_handle = FreeLibrary(_handle))
#define GP_DLERROR()							"Windows Error"
#else
#include <dirent.h>
#include <dlfcn.h>
#define GP_DLOPEN(_handle, _filename)		(_handle = dlopen(_filename, RTLD_LAZY))
#define GP_DLSYM(_pointer, _handle, _funcname)	(_pointer = dlsym(_handle, _funcname))
#define GP_DLCLOSE(_handle)			(dlclose(_handle))
#define GP_DLERROR()				(dlerror())
#endif

int gpio_library_list(gpio_device_info *list, int *count);
int gpio_library_load(gpio_device *device, gpio_device_type type);
int gpio_library_close(gpio_device *device);
