#include "config.h"
#include "gpfs-err.h"
#include "gpfs-i18n.h"

#include <stdio.h>

void
gpfs_err_setv (GPFsErr *e, GPFsErrType t, const char *format, va_list args)
{
	char buf[1024];

	if (!e) {
		fprintf (stderr, _("Error (%s): "),
			 gpfs_err_type_get_name (t));
		vfprintf (stderr, format, args);
		fputc ('\n', stderr);
		return;
	}

	memset (buf, 0, sizeof (buf));
	vsnprintf (buf, sizeof (buf) - 1, format, args);
	e->msg = strdup (buf);
}

void
gpfs_err_set (GPFsErr *e, GPFsErrType t, const char *format, ...)
{
	va_list args;

	va_start (args, format);
	gpfs_err_setv (e, t, format, args);
	va_end (args);
}

unsigned int
gpfs_err_occurred (GPFsErr *e)
{
	return ((e) && (e->t != GPFS_ERR_TYPE_NONE)) ? 1 : 0;
}
