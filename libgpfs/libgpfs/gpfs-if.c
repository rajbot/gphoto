#include "config.h"
#include "gpfs-if.h"
#include "gpfs-i18n.h"

#include <stdlib.h>
#include <string.h>

#define CNV(i,e) {							\
	if (!i) {							\
		gpfs_err_set (e, GPFS_ERR_TYPE_BAD_PARAMETERS,		\
			    _("You need to supply an interface."));	\
		return;							\
	}								\
}

#define CN0(i,e) {							\
	if (!i) {							\
		gpfs_err_set (e, GPFS_ERR_TYPE_BAD_PARAMETERS,		\
			    _("You need to supply an interface."));	\
		return 0;						\
	}								\
}

struct _GPFsIf {
	char *name;

	unsigned int ref_count;

	/* Functions */
	GPFsIfFuncCountInfo f_count_info; void *f_data_count_info;
	GPFsIfFuncGetInfo   f_get_info  ; void *f_data_get_info  ;
	GPFsIfFuncSetInfo   f_set_info  ; void *f_data_set_info  ;
	GPFsIfFuncRead      f_read      ; void *f_data_read      ;
};

GPFsIf *
gpfs_if_new (const char *name)
{
	GPFsIf *i;

	i = malloc (sizeof (GPFsIf));
	if (!i)
		return NULL;
	memset (i, 0, sizeof (GPFsIf));
	i->name = strdup (name);
	i->ref_count = 1;

	return i;
};

void
gpfs_if_ref (GPFsIf *i)
{
	if (!i)
		return;
	i->ref_count++;
}

void
gpfs_if_unref (GPFsIf *i)
{
	if (!i)
		return;
	if (!--i->ref_count) {
		free (i->name);
		free (i);
	}
}

const char *
gpfs_if_get_name (GPFsIf *i)
{
	return (i ? i->name : NULL);
}

unsigned int
gpfs_if_count_info (GPFsIf *i, GPFsErr *e)
{
	CN0 (i, e);

	if (!i->f_count_info) {
		gpfs_err_set (e, GPFS_ERR_TYPE_NOT_SUPPORTED,
			    _("This interface doesn't supply information."));
		return 0;
	}

	return i->f_count_info (i, e, i->f_data_count_info);
}

void
gpfs_if_get_info (GPFsIf *i, GPFsErr *e, unsigned int n, GPFsInfo *info)
{
	CNV (i, e);

	if (!i->f_get_info) {
		gpfs_err_set (e, GPFS_ERR_TYPE_NOT_SUPPORTED,
			_("The interface '%s' doesn't support "
			  "getting pieces of information."),
			gpfs_if_get_name (i));
		return;
	}

	i->f_get_info (i, e, n, info, i->f_data_get_info);
}

void
gpfs_if_set_info (GPFsIf *i, GPFsErr *e, unsigned int n, GPFsInfo *info)
{
	CNV (i, e);

	if (!i->f_set_info) {
		gpfs_err_set (e, GPFS_ERR_TYPE_NOT_SUPPORTED,
			_("The interface '%s' doesn't support "
			  "setting pieces of information."),
			gpfs_if_get_name (i));
		return;
	}

	i->f_set_info (i, e, n, info, i->f_data_set_info);
}

void
gpfs_if_read (GPFsIf *i, GPFsErr *e, GPFsIfFuncReadCb f, void *d)
{
	CNV (i, e);

	if (!i->f_read) {
		gpfs_err_set (e, GPFS_ERR_TYPE_NOT_SUPPORTED,
			    _("This interface doesn't support reading."));
		return;
	}

	i->f_read (i, e, f, d, i->f_data_read);
}

void
gpfs_if_set_func_count_info (GPFsIf *i, GPFsIfFuncCountInfo f, void *f_data)
{
	if (!i) return;
	i->f_count_info = f;
	i->f_data_count_info = f_data;
}

void
gpfs_if_get_func_count_info (GPFsIf *i, GPFsIfFuncCountInfo *f, void **f_data)
{
	if (!i) return;
	if (f) *f = i->f_count_info;
	if (f_data) *f_data = i->f_data_count_info;
}

void
gpfs_if_set_func_get_info (GPFsIf *i, GPFsIfFuncGetInfo f, void *f_data)
{
	if (!i) return;
	i->f_get_info = f;
	i->f_data_get_info = f_data;
}

void
gpfs_if_get_func_get_info (GPFsIf *i, GPFsIfFuncGetInfo *f, void **f_data)
{
	if (!i) return;
	if (f) *f = i->f_get_info;
	if (f_data) *f_data = i->f_data_get_info;
}

void
gpfs_if_set_func_read (GPFsIf *i, GPFsIfFuncRead f, void *f_data)
{
	if (!i)
		return;
	i->f_read = f;
	i->f_data_read = f_data;
}

void
gpfs_if_get_func_read (GPFsIf *i, GPFsIfFuncRead *f, void **f_data)
{
	if (!i || !f || !f_data)
		return;
	*f = i->f_read;
	*f_data = i->f_data_read;
}
