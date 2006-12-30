#include "config.h"
#include "gpfs-if.h"
#include "gpfs-i18n.h"
#include "gpfs-obj.h"

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
	GPFsObj parent;

	/* Functions */
	GPFsIfFuncRead f_read; void *f_data_read;
};

GPFsIf *
gpfs_if_new (void)
{
	GPFsObj *o;

	o = gpfs_obj_new (sizeof (GPFsIf));
	if (!o) return NULL;
	return (GPFsIf *) o;
};

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
