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

#define CNN(i,e) {							\
	if (!i) {							\
		gpfs_err_set (e, GPFS_ERR_TYPE_BAD_PARAMETERS,		\
			_("You need to supply an interface."));		\
		return NULL;						\
	}								\
}

struct _GPFsIf {
	char *name;

	unsigned int ref_count;

	/* Functions */
	GPFsIfFuncCountProp f_count_prop; void *f_data_count_prop;
	GPFsIfFuncGetProp   f_get_prop  ; void *f_data_get_prop  ;
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
gpfs_if_count_prop (GPFsIf *i, GPFsErr *e)
{
	CN0 (i, e);

	if (!i->f_count_prop) {
		gpfs_err_set (e, GPFS_ERR_TYPE_NOT_SUPPORTED,
			    _("This interface doesn't have properties."));
		return 0;
	}

	return i->f_count_prop (i, e, i->f_data_count_prop);
}

GPFsProp *
gpfs_if_get_prop (GPFsIf *i, GPFsErr *e, unsigned int n)
{
	CNN (i, e);

	if (!i->f_get_prop) {
		gpfs_err_set (e, GPFS_ERR_TYPE_NOT_SUPPORTED,
			_("The interface '%s' doesn't support "
			  "getting properties."),
			gpfs_if_get_name (i));
		return NULL;
	}

	return i->f_get_prop (i, e, n, i->f_data_get_prop);
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
gpfs_if_set_func_count_prop (GPFsIf *i, GPFsIfFuncCountProp f, void *f_data)
{
	if (!i) return;
	i->f_count_prop = f;
	i->f_data_count_prop = f_data;
}

void
gpfs_if_get_func_count_prop (GPFsIf *i, GPFsIfFuncCountProp *f, void **f_data)
{
	if (!i) return;
	if (f) *f = i->f_count_prop;
	if (f_data) *f_data = i->f_data_count_prop;
}

void
gpfs_if_set_func_get_prop (GPFsIf *i, GPFsIfFuncGetProp f, void *f_data)
{
	if (!i) return;
	i->f_get_prop = f;
	i->f_data_get_prop = f_data;
}

void
gpfs_if_get_func_get_prop (GPFsIf *i, GPFsIfFuncGetProp *f, void **f_data)
{
	if (!i) return;
	if (f) *f = i->f_get_prop;
	if (f_data) *f_data = i->f_data_get_prop;
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
