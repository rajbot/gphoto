
/* Windows Portability
   ------------------------------------------------------------------ */

#ifdef WIN32

#include <windows.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <direct.h>

#ifdef IOLIBS
#undef IOLIBS
#endif
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
} GPPORTWINDIR;

/* Sleep functionality */
#define	GP_SYSTEM_SLEEP(_ms)		Sleep(_ms)

/* Directory-oriented functions */
#define GP_SYSTEM_DIR		        GPPORTWINDIR *
#define GP_SYSTEM_DIRENT		WIN32_FIND_DATA *
#define GP_SYSTEM_DIR_DELIM		'\\'


#else

/* POSIX Portability
   ------------------------------------------------------------------ */

/* yummy. :) */

#include <sys/types.h>
#include <dirent.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>

/* Sleep functionality */
#define	GP_SYSTEM_SLEEP(_ms)		usleep((_ms)*1000)

/* Directory-oriented functions */
#define GP_SYSTEM_DIR                   DIR *
#define GP_SYSTEM_DIRENT		struct dirent *
#ifdef OS2
#define GP_SYSTEM_DIR_DELIM		'\\'
#else
#define GP_SYSTEM_DIR_DELIM		'/'
#endif /* OS2 */

#endif /* else */

int		 GP_SYSTEM_MKDIR	(const char *dirname);
int              GP_SYSTEM_RMDIR        (const char *dirname);
GP_SYSTEM_DIR	 GP_SYSTEM_OPENDIR	(const char *dirname);
GP_SYSTEM_DIRENT GP_SYSTEM_READDIR	(GP_SYSTEM_DIR d);
const char*	 GP_SYSTEM_FILENAME	(GP_SYSTEM_DIRENT de);
int		 GP_SYSTEM_CLOSEDIR	(GP_SYSTEM_DIR dir);
int		 GP_SYSTEM_IS_FILE	(const char *filename);
int		 GP_SYSTEM_IS_DIR	(const char *dirname);
