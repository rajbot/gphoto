#ifndef __GP_ERR_H__
#define __GP_ERR_H__

#include <string.h>
#include <stdarg.h>

#include <libgpfs/gpfs-err-type.h>

typedef struct _GPFsErr GPFsErr;
struct _GPFsErr {
	GPFsErrType t;
	char *msg;
};

void gpfs_err_set  (GPFsErr *, GPFsErrType, const char *, ...);
void gpfs_err_setv (GPFsErr *, GPFsErrType, const char *, va_list args);

unsigned int gpfs_err_occurred (GPFsErr *);

#define gpfs_err_init(e)     {if (e) memset (e, 0, sizeof (GPFsErr));}
#define gpfs_err_clear(e)    {if (e) free (e->msg);}

#endif
