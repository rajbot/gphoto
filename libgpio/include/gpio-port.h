
/* Windows Portability
   ------------------------------------------------------------------ */

#ifdef WIN32

#include <windows.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <direct.h>

#define IOLIBS			"."
#define strcasecmp		_stricmp
#define snprintf		_snprintf

/* Work-around for readdir() */
typedef struct {
	HANDLE handle;
	int got_first;
	WIN32_FIND_DATA search;
	char dir[1024];
	char drive[32][2];
	int  drive_count;
	int  drive_index;
} GPIOWINDIR;

/* Sleep functionality */
#define	GPIO_SLEEP(_ms)		Sleep(_ms)

/* Dynamic library functions */
#define GPIO_DLOPEN(_filename)		LoadLibrary(_filename)
#define GPIO_DLSYM(_handle, _funcname)	GetProcAddress(_handle, _funcname)
#define GPIO_DLCLOSE(_handle)		FreeLibrary(_handle)
#define GPIO_DLERROR()			"Windows Error"

/* Directory-oriented functions */
#define GPIO_DIR		GPIOWINDIR *
#define GPIO_DIRENT		WIN32_FIND_DATA *
#define GPIO_DIR_DELIM		'\\'


#else

/* POSIX Portability
   ------------------------------------------------------------------ */

/* yummy. :) */

#include <dirent.h>
#include <dlfcn.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* Sleep functionality */
#define	GPIO_SLEEP(_ms)		usleep(_ms*1000)

/* Dynamic library functions */
#define GPIO_DLOPEN(_filename)		dlopen(_filename, RTLD_LAZY)
#define GPIO_DLSYM(_handle, _funcname)	dlsym(_handle, _funcname)
#define GPIO_DLCLOSE(_handle)		dlclose(_handle)
#define GPIO_DLERROR()			dlerror()

/* Directory-oriented functions */
#define GPIO_DIR		DIR *
#define GPIO_DIRENT		struct dirent *
#ifdef OS2
#define GPIO_DIR_DELIM		'\\'
#else
#define GPIO_DIR_DELIM		'/'
#endif /* OS2 */

#endif /* else */

int		GPIO_MKDIR		(char *dirname);
GPIO_DIR	GPIO_OPENDIR	(char *dirname);
GPIO_DIRENT	GPIO_READDIR	(GPIO_DIR d);
char*		GPIO_FILENAME	(GPIO_DIRENT de);
int		GPIO_CLOSEDIR	(GPIO_DIR dir);
int		GPIO_IS_FILE	(char *filename);
int		GPIO_IS_DIR	(char *dirname);
