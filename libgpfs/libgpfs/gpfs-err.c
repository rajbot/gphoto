#include <config.h>
#include "gpfs-err.h"

#include <stdio.h>

#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (GETTEXT_PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

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
