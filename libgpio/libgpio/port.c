#include <gpio.h>

/* Windows Portability
   ------------------------------------------------------------------ */
#ifdef WIN32

GPIO_DIR GPIO_OPENDIR (char *dirname) {

	GPIOWINDIR *d;
	char dirn[1024];

	d = (GPIOWINDIR*)malloc(sizeof(GPIOWINDIR));

	/* Append the wildcard */
	strcpy(dirn, dirname);
	strcat(dirn, "\\*");

	d->got_first = 0;
	d->handle = FindFirstFile(dirn, &(d->search));

	if ((!d->handle) || (d->handle == INVALID_HANDLE_VALUE)) {
			free(d);
			return NULL;
	}

	return (d);
}

GPIO_DIRENT GPIO_READDIR (GPIO_DIR d) {
	if (d->got_first == 0) {
		d->got_first = 1;
		return (&(d->search));
	}

	if (!FindNextFile(d->handle, &(d->search)))
			return (NULL);

	return (&(d->search));
}

char *GPIO_FILENAME (GPIO_DIRENT de) {

	return (de->cFileName);
}

int  GPIO_CLOSEDIR (GPIO_DIR d) {
	FindClose(d->handle);
	free(d);
	return (1);
}

int GPIO_IS_FILE (char *filename) {

	return (GPIO_OK);
}

int GPIO_IS_DIR (char *dirname) {

	return (GPIO_OK);
}


#else

GPIO_DIR GPIO_OPENDIR (char *dirname) {
	return (opendir(dirname));
}

GPIO_DIRENT GPIO_READDIR (GPIO_DIR d) {
	return (readdir(d));
}

char *GPIO_FILENAME (GPIO_DIRENT de) {
	return (de->d_name);
}

int GPIO_CLOSEDIR (GPIO_DIR dir) {
	closedir(dir);
}

int GPIO_IS_FILE (char *filename) {
	struct stat st;

	if (stat(filename, &st)==0)
		return 0;
	return (!S_ISDIR(st.st_mode));
}

int GPIO_IS_DIR (char *dirname) {
	struct stat st;

	if (stat(dirname, &st)==0)
		return 0;
	return (S_ISDIR(st.st_mode));
}
#endif
