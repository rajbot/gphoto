#include <sys/types.h>
#include <stdio.h>

#include "gpio.h"
#include "library.h"

extern int device_count;
extern gpio_device_info device_list[];
void *device_lh;

int gpio_library_is_valid (char *filename) {

	void *lh;

	if (GP_DLOPEN(lh, filename)==NULL) {
		gpio_debug_printf("%s is not a library (%s) ", filename, GP_DLERROR());
		return (GPIO_ERROR);
	}

	gpio_debug_printf("%s is a library ", filename);
	GP_DLCLOSE(lh);

	return (GPIO_OK);
}

int gpio_library_list_load(char *filename, int loaded[], gpio_device_info *list, int *count) {

	void *lh;
	int type, x;
	gpio_ptr_list lib_list;
	gpio_ptr_type lib_type;
	int old_count = *count;

	if (GP_DLOPEN(lh, filename)==NULL)
		return (GPIO_ERROR);

	GP_DLSYM(lib_type, lh, "gpio_library_type");
	GP_DLSYM(lib_list, lh, "gpio_library_list");

	if ((!list) || (!lib_type)) {
		gpio_debug_printf("%s ", GP_DLERROR());
		GP_DLCLOSE(lh);
		return (GPIO_ERROR);
	}

	type = lib_type();

	if (loaded[type] == 1) {
		gpio_debug_printf("%s (%i) already loaded ", filename, type);
		GP_DLCLOSE(lh);
		return (GPIO_ERROR);
	} else {
		loaded[type] = 1;
	}

	if (lib_list(list, count)==GPIO_ERROR)
		gpio_debug_printf("%s could not list devices ", filename);

	/* copy in the library path */
	for (x=old_count; x<(*count); x++)
		strcpy(list[x].library_filename, filename);

	GP_DLCLOSE(lh);
	return (GPIO_OK);
}

int gpio_library_list (gpio_device_info *list, int *count) {

	DIR *d;
	int loaded[256];
	int x;
	char buf[1024];
	struct dirent *de;

	*count = 0;

	for (x=0;x<256; x++)
		loaded[x]=0;

	/* Look for available camera libraries */
	d = opendir(IOLIBS);
	if (!d) {
		gpio_debug_printf("couldn't open %s ", IOLIBS);
		return GPIO_ERROR;
	}

	do {
	   /* Read each entry */
	   de = readdir(d);
	   if (de) {
#if defined(OS2) || defined(WINDOWS)
		sprintf(buf, "%s\\%s", IOLIBS, de->d_name);
#else
		sprintf(buf, "%s/%s", IOLIBS, de->d_name);
#endif
		if (gpio_library_is_valid(buf) == GPIO_OK)
			gpio_library_list_load(buf, loaded, list, count);
	   }
	} while (de);

	return (GPIO_OK);
}

int gpio_library_load (gpio_device *device, gpio_device_type type) {

	int x=0;
	gpio_ptr_operations ops_func;

	for (x=0; x<device_count; x++) {
		if (device_list[x].type == type) {
			/* Open the correct library */
			GP_DLOPEN(device->library_handle, device_list[x].library_filename);
			if (!device->library_handle) {
				gpio_debug_printf (" %s %s ", device_list[x].library_filename,
					GP_DLERROR());
				return (GPIO_ERROR);
			}

			/* Load the operations */
			GP_DLSYM(ops_func, device->library_handle, "gpio_library_operations");
			if (!ops_func) {
				gpio_debug_printf (" %s %s ", device_list[x].library_filename,
					GP_DLERROR());
				GP_DLCLOSE(device->library_handle);
				return (GPIO_ERROR);
			}
			device->ops = ops_func();
			return (GPIO_OK);
		}
	}
	return (GPIO_ERROR);
}

int gpio_library_close (gpio_device *device) {

	GP_DLCLOSE(device->library_handle);
}
