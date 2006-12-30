#include "config.h"
#include "gpfs-bag.h"
#include "gpfs-i18n.h"
#include "gpfs-obj.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define CN0(b,e) {							\
	if (!b) {							\
		gpfs_err_set (e, GPFS_ERR_TYPE_BAD_PARAMETERS,		\
			_("You need to supply a property bag."));	\
		return 0;						\
	}								\
}

#define CNN(b,e) {							\
	if (!b) {							\
		gpfs_err_set (e, GPFS_ERR_TYPE_BAD_PARAMETERS,		\
			_("You need to supply a property bag."));	\
		return 0;						\
	}								\
}

#define CNV(b,e) {							\
	if (!b) {							\
		gpfs_err_set (e, GPFS_ERR_TYPE_BAD_PARAMETERS,		\
			_("You need to supply a property bag."));	\
		return;							\
	}								\
}

struct _GPFsBag {
	GPFsObj parent;

	GPFsBagFuncCount      f_prop_count ; void *f_data_prop_count ;
	GPFsBagFuncCount      f_bag_count  ; void *f_data_bag_count  ;
	GPFsBagFuncBagAdd     f_bag_add    ; void *f_data_bag_add    ;
	GPFsBagFuncPropAdd    f_prop_add   ; void *f_data_prop_add   ;
	GPFsBagFuncRemove     f_bag_remove ; void *f_data_bag_remove ;
	GPFsBagFuncRemove     f_prop_remove; void *f_data_prop_remove;
	GPFsBagFuncBagGet     f_bag_get    ; void *f_data_bag_get    ;
	GPFsBagFuncPropGet    f_prop_get   ; void *f_data_prop_get   ;

	GPFsBag **b;
	unsigned int b_count;

	GPFsProp **p;
	unsigned int p_count;
};

void
gpfs_bag_set_func_prop_count (GPFsBag *b, GPFsBagFuncCount f, void *f_data)
{
	if (!b) return;
	b->f_prop_count = f;
	b->f_data_prop_count = f_data;
}

GPFsBag *
gpfs_bag_new (void)
{
	GPFsObj *o = gpfs_obj_new (sizeof (GPFsBag));
	if (!o) return NULL;
	return (GPFsBag *) o;
}

unsigned int
gpfs_bag_bag_count (GPFsBag *b, GPFsErr *e)
{
	CN0(b,e);
	if (!b->f_bag_count) return b->b_count;
	return b->f_bag_count (b, e, b->f_data_bag_count);
}

unsigned int
gpfs_bag_prop_count (GPFsBag *b, GPFsErr *e)
{
	CN0(b,e);
	if (!b->f_prop_count) return b->p_count;
	return b->f_prop_count (b, e, b->f_data_prop_count);
}

GPFsBag *
gpfs_bag_bag_get (GPFsBag *b, GPFsErr *e, unsigned int n)
{
	CNN(b,e);
	if (!b->f_bag_get) return (n > b->b_count) ? NULL : b->b[n];
	return b->f_bag_get (b, e, n, b->f_data_bag_get);
}

GPFsProp *
gpfs_bag_prop_get (GPFsBag *b, GPFsErr *e, unsigned int n)
{
	CNN(b,e);

	if (!b->f_prop_get) return (n > b->p_count) ? NULL : b->p[n];
	return b->f_prop_get (b, e, n, b->f_data_prop_get);
}

void
gpfs_bag_prop_remove (GPFsBag *b, GPFsErr *e, unsigned int n)
{
	CNV(b,e);
	if (!b->f_prop_remove) {
		gpfs_err_set (e, GPFS_ERR_TYPE_NOT_SUPPORTED,
			_("This property bag does not support removing "
			  "properties."));
		return;
	}
	b->f_prop_remove (b, e, n, b->f_data_prop_remove);
}

void
gpfs_bag_bag_remove (GPFsBag *b, GPFsErr *e, unsigned int n)
{
	CNV(b,e);
	if (!b->f_bag_remove) {
		gpfs_err_set (e, GPFS_ERR_TYPE_NOT_SUPPORTED,
			_("This property bag does not support removing "
			  "property bags."));
		return;
	}
	b->f_bag_remove (b, e, n, b->f_data_bag_remove);
}

void
gpfs_bag_bag_add (GPFsBag *b, GPFsErr *e, GPFsBag *n)
{
	CNV(b,e);
	if (!b->f_bag_add) {
		gpfs_err_set (e, GPFS_ERR_TYPE_NOT_SUPPORTED,
			_("This property bag does not support adding "
			  "property bags."));
		return;
	}
	b->f_bag_add (b, e, n, b->f_data_bag_add);
	if (!gpfs_err_occurred (e)) gpfs_bag_bag_add_impl (b, n);
}

void
gpfs_bag_bag_add_impl (GPFsBag *b, GPFsBag *n)
{
	GPFsBag **b_new;

	if (!b || !n) return;
	b_new = realloc (b->b, sizeof (GPFsBag *) * (b->b_count + 1));
	if (!b_new) return;
	b->b = b_new;
	b->b[b->b_count] = n;
	b->b_count++;
}

void
gpfs_bag_prop_add_impl (GPFsBag *b, GPFsProp *p)
{
	GPFsProp **p_new;
	if (!b || !p) return;
	p_new = realloc (b->p, sizeof (GPFsProp *) * (b->p_count + 1));
	if (!p_new) return;
	b->p = p_new;
	b->p[b->p_count] = p;
	b->p_count++;
}

void
gpfs_bag_prop_add (GPFsBag *b, GPFsErr *e, GPFsProp *p)
{
	CNV(b,e);
	if (!b->f_prop_add) {
		gpfs_err_set (e, GPFS_ERR_TYPE_NOT_SUPPORTED,
			_("This property bag does not support adding "
			  "properties."));
		return;
	}
	b->f_prop_add (b, e, p, b->f_data_prop_add);
	gpfs_bag_prop_add_impl (b, p);
}

void
gpfs_bag_dump (GPFsBag *b)
{
	unsigned int i, n;
	GPFsProp *p;
	GPFsBag *bag;

	printf ("Dumping contents of property bag '%s':\n",
		gpfs_obj_get_name (GPFS_OBJ (b), NULL));
	printf ("(1) Properties\n");
	n = gpfs_bag_prop_count (b, NULL);
	for (i = 0; i < n; i++) {
		p = gpfs_bag_prop_get (b, NULL, i);
		if (p) gpfs_prop_dump (p);
	}
	printf ("(2) Property bags\n");
	n = gpfs_bag_bag_count (b, NULL);
	for (i = 0; i < n; i++) {
		bag = gpfs_bag_bag_get (b, NULL, i);
		if (bag) gpfs_bag_dump (bag);
	}
}

void
gpfs_bag_set_func_prop_get (GPFsBag *b, GPFsBagFuncPropGet f, void *f_data)
{
	if (!b) return;
	b->f_prop_get = f;
	b->f_data_prop_get = f_data;
}
