#include <stdio.h>
#include <gpio.h>

/* Windows Portability
   ------------------------------------------------------------------ */
#ifdef WIN32

void gpio_win_convert_path (char *path) {

	int x;

	if (strchr(path, '\\'))
		/* already converted */
		return;

	if (path[0] != '.') {
		path[0] = path[1];
		path[1] = ':';
		path[2] = '\\';
	}

	for (x=0; x<strlen(path); x++)
		if (path[x] == '/')
			path[x] = '\\';
}

int GPIO_MKDIR (char *dirname) {

	if (_mkdir(dirname) < 0)
		return (GPIO_ERROR);
	return (GPIO_OK);
}

GPIO_DIR GPIO_OPENDIR (char *dirname) {

	GPIOWINDIR *d;
	DWORD dr;
	int x;

	d = (GPIOWINDIR*)malloc(sizeof(GPIOWINDIR));
	d->handle = INVALID_HANDLE_VALUE;
	d->got_first = 0;
	strcpy(d->dir, dirname);
	d->drive_count = 0;
	d->drive_index = 0;

	dr = GetLogicalDrives();
	
	for (x=0; x<32; x++) {
		if ((dr >> x) & 0x0001) {
			sprintf(d->drive[d->drive_count], "%c", 'A' + x);
			d->drive_count += 1;
		}	
	}

	return (d);
}

GPIO_DIRENT GPIO_READDIR (GPIO_DIR d) {

	char dirn[1024];

	if (strcmp(d->dir, "/")==0) {
		if (d->drive_index == d->drive_count)
			return (NULL);
		strcpy(d->search.cFileName, d->drive[d->drive_index]);
		d->drive_index += 1;
		return (&(d->search));
	}	


	/* Append the wildcard */
	
	strcpy(dirn, d->dir);
	gpio_win_convert_path(dirn);
	
	if (dirn[strlen(dirn)-1] != '\\')
		strcat(dirn, "\\");
	strcat(dirn, "*");


	if (d->handle == INVALID_HANDLE_VALUE) {
		d->handle = FindFirstFile(dirn, &(d->search));
		if (d->handle == INVALID_HANDLE_VALUE)
			return NULL;
	} else {
		if (!FindNextFile(d->handle, &(d->search)))
			return NULL;
	}

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

	struct stat st;

	gpio_win_convert_path(filename);

	if (stat(filename, &st)!=0)
		return 0;
	return (st.st_mode & _S_IFREG);
}

int GPIO_IS_DIR (char *dirname) {

	struct stat st;

	if (strlen(dirname) <= 3)
		return 1;

	gpio_win_convert_path(dirname);

	if (stat(dirname, &st)!=0)
		return 0;
	return (st.st_mode & _S_IFDIR);
}


#else

int GPIO_MKDIR (char *dirname) {

	if (mkdir(dirname, 0700)<0)
		return (GPIO_ERROR);
	return (GPIO_OK);
}

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
	return (GPIO_OK);
}

int GPIO_IS_FILE (char *filename) {
	struct stat st;

	if (stat(filename, &st)!=0)
		return 0;
	return (!S_ISDIR(st.st_mode));
}

int GPIO_IS_DIR (char *dirname) {
	struct stat st;

	if (stat(dirname, &st)!=0)
		return 0;
	return (S_ISDIR(st.st_mode));
}
#endif
