#include <sys/types.h>
#include <stdio.h>

#include "gpio.h"
#include "library.h"

extern int device_count;
extern int glob_debug_level;
extern gpio_device_info device_list[];
void *device_lh;

int gpio_library_is_valid (char *filename) {

	void *lh;

	if ((lh = GPIO_DLOPEN(filename))==NULL) {
		gpio_debug_printf(GPIO_DEBUG_LOW, glob_debug_level, 
			"%s is not a library (%s) ", filename, GPIO_DLERROR());
		return (GPIO_ERROR);
	}

	gpio_debug_printf(GPIO_DEBUG_LOW, glob_debug_level, "%s is a library ", filename);
	GPIO_DLCLOSE(lh);

	return (GPIO_OK);
}

int gpio_library_list_load(char *filename, int loaded[], gpio_device_info *list, int *count) {

	void *lh;
	int type, x;
	gpio_ptr_type lib_type;
	gpio_ptr_list lib_list;
	int old_count = *count;

	if ((lh = GPIO_DLOPEN(filename))==NULL)
		return (GPIO_ERROR);

	lib_type = (gpio_ptr_type)GPIO_DLSYM(lh, "gpio_library_type");
	lib_list = (gpio_ptr_list)GPIO_DLSYM(lh, "gpio_library_list");

	if ((!list) || (!lib_type)) {
		gpio_debug_printf(GPIO_DEBUG_LOW, glob_debug_level, 
			"could not find type/list symbols: %s ", GPIO_DLERROR());
		GPIO_DLCLOSE(lh);
		return (GPIO_ERROR);
	}

	type = lib_type();

	if (loaded[type] == 1) {
		gpio_debug_printf(GPIO_DEBUG_LOW, glob_debug_level, 
			"%s (%i) already loaded ", filename, type);
		GPIO_DLCLOSE(lh);
		return (GPIO_ERROR);
	} else {
		loaded[type] = 1;
	}

	if (lib_list(list, count)==GPIO_ERROR)
		gpio_debug_printf(GPIO_DEBUG_LOW, glob_debug_level, 
			"%s could not list devices ", filename);

	gpio_debug_printf(GPIO_DEBUG_LOW, glob_debug_level,
		"Loaded these devices from %s:", filename);
	/* copy in the library path */
	for (x=old_count; x<(*count); x++) {
		gpio_debug_printf(GPIO_DEBUG_LOW, glob_debug_level,
			"\t%s path=\"%s\"", list[x].name, list[x].path);
		strcpy(list[x].library_filename, filename);
	}

	GPIO_DLCLOSE(lh);
	return (GPIO_OK);
}

int gpio_library_list (gpio_device_info *list, int *count) {

	GPIO_DIR d;
	GPIO_DIRENT de;
	int loaded[256];
	int x;
	char buf[1024];

	*count = 0;

	for (x=0;x<256; x++)
		loaded[x]=0;

	/* Look for available camera libraries */
	d = GPIO_OPENDIR(IOLIBS);
	if (!d) {
		gpio_debug_printf(GPIO_DEBUG_LOW, glob_debug_level, 
			"couldn't open %s ", IOLIBS);
		return GPIO_ERROR;
	}

	do {
	   /* Read each entry */
	   de = GPIO_READDIR(d);
	   if (de) {
#if defined(OS2) || defined(WIN32)
		sprintf(buf, "%s\\%s", IOLIBS, GPIO_FILENAME(de));
#else
		sprintf(buf, "%s/%s", IOLIBS, GPIO_FILENAME(de));
#endif
		if (gpio_library_is_valid(buf) == GPIO_OK)
			gpio_library_list_load(buf, loaded, list, count);
	   }
	} while (de);

	GPIO_CLOSEDIR(d);

	return (GPIO_OK);
}

int gpio_library_load (gpio_device *device, gpio_device_type type) {

	int x=0;
	gpio_ptr_operations ops_func;

	for (x=0; x<device_count; x++) {
		if (device_list[x].type == type) {
			/* Open the correct library */
			device->library_handle = GPIO_DLOPEN(device_list[x].library_filename);
			if (!device->library_handle) {
				gpio_debug_printf(GPIO_DEBUG_LOW, glob_debug_level, 
					"bad handle: %s %s ", 
					device_list[x].library_filename, GPIO_DLERROR());
				return (GPIO_ERROR);
			}

			/* Load the operations */
			ops_func = (gpio_ptr_operations)GPIO_DLSYM(device->library_handle, "gpio_library_operations");
			if (!ops_func) {
				gpio_debug_printf(GPIO_DEBUG_LOW, glob_debug_level,
					"can't load ops: %s %s ", 
					device_list[x].library_filename, GPIO_DLERROR());
				GPIO_DLCLOSE(device->library_handle);
				return (GPIO_ERROR);
			}
			device->ops = ops_func();
			return (GPIO_OK);
		}
	}
	return (GPIO_ERROR);
}

int gpio_library_close (gpio_device *device) {

	GPIO_DLCLOSE(device->library_handle);

	return (GPIO_OK);
}
