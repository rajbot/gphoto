#include <sys/types.h>
#include <stdio.h>

#include "gpio.h"
#include "library.h"

extern int device_count;
extern gpio_device_info device_list[];
void *device_lh;

int gpio_library_is_valid (char *filename) {

	void *lh;

	if ((lh = GP_DLOPEN(filename))==NULL) {
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
	gpio_ptr_type lib_type;
	gpio_ptr_list lib_list;
	int old_count = *count;

	if ((lh = GP_DLOPEN(filename))==NULL)
		return (GPIO_ERROR);

	lib_type = (gpio_ptr_type)GP_DLSYM(lh, "gpio_library_type");
	lib_list = (gpio_ptr_list)GP_DLSYM(lh, "gpio_library_list");

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

	GP_DIR d;
	GP_DIRENT de;
	int loaded[256];
	int x;
	char buf[1024];

	*count = 0;

	for (x=0;x<256; x++)
		loaded[x]=0;

	/* Look for available camera libraries */
	d = GP_OPENDIR(IOLIBS);
	if (!d) {
		gpio_debug_printf("couldn't open %s ", IOLIBS);
		return GPIO_ERROR;
	}

	do {
	   /* Read each entry */
	   de = GP_READDIR(d);
	   if (de) {
#if defined(OS2) || defined(WIN32)
		sprintf(buf, "%s\\%s", IOLIBS, GP_FILENAME(de->d_name));
#else
		sprintf(buf, "%s/%s", IOLIBS, GP_FILENAME(de->d_name));
#endif
		if (gpio_library_is_valid(buf) == GPIO_OK)
			gpio_library_list_load(buf, loaded, list, count);
	   }
	} while (de);

	GP_CLOSEDIR(d);

	return (GPIO_OK);
}

int gpio_library_load (gpio_device *device, gpio_device_type type) {

	int x=0;
	gpio_ptr_operations ops_func;

	for (x=0; x<device_count; x++) {
		if (device_list[x].type == type) {
			/* Open the correct library */
			device->library_handle = GP_DLOPEN(device_list[x].library_filename);
			if (!device->library_handle) {
				gpio_debug_printf (" %s %s ", device_list[x].library_filename,
					GP_DLERROR());
				return (GPIO_ERROR);
			}

			/* Load the operations */
			ops_func = GP_DLSYM(device->library_handle, "gpio_library_operations");
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
