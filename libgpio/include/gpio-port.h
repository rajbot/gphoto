
/* Windows Portability
   ------------------------------------------------------------------ */

#ifdef WIN32

#include <windows.h>
#include <string.h>

/* Libraries go in the current directory */
#define IOLIBS ".\\"

/* Work-around for readdir() */
typedef struct {
	HANDLE h;
	int got_first;
	WIN32_FIND_DATA first;
} GPWINDIR;

/* Dynamic library functions */
#define GP_DLOPEN(_filename)		LoadLibrary(_filename)
#define GP_DLSYM(_handle, _funcname)	GetProcAddress(_handle, _funcname)
#define GP_DLCLOSE(_handle)		FreeLibrary(_handle)
#define GP_DLERROR()			"Windows Error"

/* Directory-oriented functions */
#define GP_DIR				GPWINDIR *
#define GP_DIRENT			WIN32_FIND_DATA

#else

/* POSIX Portability
   ------------------------------------------------------------------ */

/* yummy. :) */

#include <dirent.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* Dynamic library functions */
#define GP_DLOPEN(_filename)		dlopen(_filename, RTLD_LAZY)
#define GP_DLSYM(_handle, _funcname)	dlsym(_handle, _funcname)
#define GP_DLCLOSE(_handle)		dlclose(_handle)
#define GP_DLERROR()			dlerror()

/* Directory-oriented functions */
#define GP_DIR				DIR*
#define GP_DIRENT			struct dirent*

#endif
