#include <config.h>
#include "gpfs-obj.h"
#include "gpfs-i18n.h"

#include <stdlib.h>

#define CN0(o,e) {							\
	if (!o) {							\
		gpfs_err_set (e, GPFS_ERR_TYPE_BAD_PARAMETERS,		\
			_("You need to supply an object."));		\
		return 0;						\
	}								\
}

#define CNV(o,e) {							\
	if (!o) {							\
		gpfs_err_set (e, GPFS_ERR_TYPE_BAD_PARAMETERS,		\
			_("You need to supply an object."));		\
		return 0;						\
	}								\
}

struct _GPFsObjPrivate {
	unsigned int ref_count;

	char *name;

	GPFsObjFuncGetName    f_get_name   ; void *f_get_name_data   ;
	GPFsObjFuncSetName    f_set_name   ; void *f_set_name_data   ;
	GPFsObjFuncBagCount   f_bag_count  ; void *f_bag_count_data  ;
	GPFsObjFuncBagGet     f_bag_get    ; void *f_bag_get_data    ;
	GPFsObjFuncBagAdd     f_bag_add    ; void *f_bag_add_data    ;
	GPFsObjFuncBagRemove  f_bag_remove ; void *f_bag_remove_data ;

	GPFsBag **bag;
	unsigned int bag_count;
};

void
gpfs_obj_init (GPFsObj *v)
{
	GPFsObj *o = v;

	memset (o, 0, sizeof (GPFsObj));
	o->priv = malloc (sizeof (GPFsObjPrivate));
	if (!o->priv)
		return;
	memset (o->priv, 0, sizeof (GPFsObjPrivate));
	o->priv->ref_count = 1;
}

void
gpfs_obj_ref (GPFsObj *v)
{
	if (v) ((GPFsObj *) v)->priv->ref_count++;
}

void
gpfs_obj_unref (GPFsObj *v)
{
	GPFsObj *o = v;

	if (!v) return;
	if (!--o->priv->ref_count) {
		if (o->f_free) o->f_free (o);
		free (o->priv->name);
		free (o);
	}
}

const char *
gpfs_obj_get_name (GPFsObj *o, GPFsErr *e)
{
	CNV(o,e);

	if (!o->priv->f_get_name) return o->priv->name;
	return o->priv->f_get_name (o, e, o->priv->f_get_name_data);
}

void
gpfs_obj_set_name (GPFsObj *o, GPFsErr *e, const char *name)
{
	if (!o->priv->f_set_name) {
		free (o->priv->name);
		o->priv->name = name ? strdup (name) : NULL;
		return;
	}
	o->priv->f_set_name (o, e, name, o->priv->f_set_name_data);
}

void
gpfs_obj_set_func_get_name (GPFsObj *o, GPFsObjFuncGetName f, void *f_data)
{
	if (!o) return;
	o->priv->f_get_name = f;
	o->priv->f_get_name_data = f_data;
}

void
gpfs_obj_get_func_get_name (GPFsObj *v, GPFsObjFuncGetName *f, void **f_data)
{
	GPFsObj *o = v;

	if (!o) return;
	if (f) *f = o->priv->f_get_name;
	if (f_data) *f_data = o->priv->f_get_name_data;
}

unsigned int
gpfs_obj_bag_count (GPFsObj *o, GPFsErr *e)
{
	CN0(o,e);

	if (!o->priv->f_bag_count) return 0;
	return o->priv->f_bag_count (o, e, o->priv->f_bag_count_data);
}

void
gpfs_obj_set_func_bag_count (GPFsObj *o, GPFsObjFuncBagCount f, void *f_data)
{
	if (!o) return;
	o->priv->f_bag_count = f;
	o->priv->f_bag_count_data = f_data;
}

void
gpfs_obj_set_func_bag_get (GPFsObj *o, GPFsObjFuncBagGet f, void *f_data)
{
	if (!o) return;
	o->priv->f_bag_get = f;
	o->priv->f_bag_get_data = f_data;
}
