#include "config.h"
#include "gpfs-cache.h"

#include <stdlib.h>
#include <string.h>

#include "gpfs-obj.h"

typedef struct _GPFsCacheIf GPFsCacheIf;
struct _GPFsCacheIf {
	GPFsIf *i;

	unsigned int limit;

	GPFsIfFuncRead f_read; void *f_data_read;

	char *data;
	unsigned int size;
};

typedef struct _GPFsCacheBag GPFsCacheBag;
struct _GPFsCacheBag {
	GPFsBag *b;

	unsigned int limit;

	GPFsBagFuncCount   f_bag_count  ; void *f_data_bag_count  ;
	GPFsBagFuncBagGet  f_bag_get    ; void *f_data_bag_get    ;
	GPFsBagFuncBagAdd  f_bag_add    ; void *f_data_bag_add    ;
	GPFsBagFuncRemove  f_bag_remove ; void *f_data_bag_remove ;

	GPFsBagFuncCount   f_prop_count ; void *f_data_prop_count ;
	GPFsBagFuncPropGet f_prop_get   ; void *f_data_prop_get   ;
	GPFsBagFuncPropAdd f_prop_add   ; void *f_data_prop_add   ;
	GPFsBagFuncRemove  f_prop_remove; void *f_data_prop_remove;

	GPFsBag **bag;
	unsigned int bag_count;

	GPFsProp **prop;
	unsigned int prop_count;
};

struct _GPFsCache {
	GPFsObj parent;

	GPFsCacheIf *i;
	unsigned int i_count;

	GPFsCacheBag *b;
	unsigned int b_count;
};

static void
gpfs_cache_free (GPFsObj *o)
{
	GPFsCache *c = (GPFsCache *) o;

	/* Remove interfaces from cache */
	while (gpfs_cache_if_count (c))
		gpfs_cache_if_remove (c, c->i[0].i);
}

GPFsCache *
gpfs_cache_new (void)
{
	GPFsObj *o;

	o = gpfs_obj_new (sizeof (GPFsCache));
	if (!o) return NULL;
	o->f_free = gpfs_cache_free;
	return (GPFsCache *) o;
}

typedef struct {
	GPFsCacheIf *c;
	GPFsIfFuncReadCb cb;
	void *cb_data;
	char cache_failed;
} ReadCbData;

static void
f_read_cb (GPFsIf *i, GPFsErr *e, const char *data, unsigned int size, void *ud)
{
	ReadCbData *d = (ReadCbData *) ud;
	char *b;

	/* Pass the data through... */
	if (d->cb)
		d->cb (i, e, data, size, d->cb_data);

	/* ... and cache it. Do not cache if we would violate the limit. */
	if (d->cache_failed)
		return;
	if (d->c->size + size > d->c->limit) {
		free (d->c->data);
		d->c->data = NULL;
		d->c->size = 0;
		d->cache_failed = 1;
		return;
	}
	b = realloc (d->c->data, sizeof (char) * (d->c->size + size));
	if (!b)
		return;
	d->c->data = b;
	memcpy (d->c->data + d->c->size, data, size);
	d->c->size += size;
}

static void
f_read (GPFsIf *i, GPFsErr *e, GPFsIfFuncReadCb cb, void *cb_data, void *ud)
{
	GPFsCache *c = (GPFsCache *) ud;
	unsigned int n;
	ReadCbData d;

	/* Sanity checks */
	if (!c)
		return;
	for (n = 0; (n < c->i_count) && (c->i[n].i != i); n++);
	if (n == c->i_count)
		return;

	/* Cached data available? */
	if (c->i[n].data) {
		cb (i, e, c->i[n].data, c->i[n].size, cb_data);
		return;
	}

	/* Get data and cache it. */
	if (!c->i[n].f_read)
		return;
	d.c = &c->i[n];
	d.cb = cb;
	d.cb_data = cb_data;
	d.cache_failed = 0;
	c->i[n].f_read (i, e, f_read_cb, &d, c->i[n].f_data_read);
}

void
gpfs_cache_if_add (GPFsCache *c, GPFsIf *i)
{
	GPFsCacheIf *in;

	if (!c || !i)
		return;

	in = realloc (c->i, sizeof (GPFsCacheIf) * (c->i_count + 1));
	if (!in)
		return;
	c->i = in;
	memset (&c->i[c->i_count], 0, sizeof (GPFsCacheIf));
	c->i[c->i_count++].i = i;
	gpfs_obj_ref (GPFS_OBJ (i));

	/* Intercept the communication */
	gpfs_if_get_func_read (i, &c->i[c->i_count - 1].f_read,
				  &c->i[c->i_count - 1].f_data_read);
	gpfs_if_set_func_read (i, f_read, c);
}

unsigned int
gpfs_cache_if_count (GPFsCache *c)
{
	return (c ? c->i_count : 0);
}

GPFsIf *
gpfs_cache_if_get (GPFsCache *c, unsigned int n)
{
	if (!c || (n >= c->i_count))
		return NULL;
	
	return c->i[n].i;
}

void
gpfs_cache_if_remove (GPFsCache *c, GPFsIf *i)
{
	unsigned int n;

	if (!c || !i)
		return;

	for (n = 0; (n < c->i_count) && (c->i[n].i != i); n++);
	if (n == c->i_count)
		return;
	gpfs_obj_unref (GPFS_OBJ (c->i[n].i));
	free (c->i[n].data);
	memmove (&c->i[n], &c->i[n + 1], c->i_count - n - 1);
	c->i_count--;
}

void
gpfs_cache_if_set_limit (GPFsCache *c, GPFsIf *i, unsigned int l)
{
	unsigned int n;

	if (!c || !i) return;
	for (n = 0; (n < c->i_count) && (c->i[n].i != i); n++);
	if (n == c->i_count) return;
	c->i[n].limit = l;
	if (c->i[n].size > l) {
		free (c->i[n].data);
		c->i[n].data = NULL;
		c->i[n].size = 0;
	}
}

unsigned int
gpfs_cache_if_get_limit (GPFsCache *c, GPFsIf *i)
{
	unsigned int n;

	if (!c || !i) return 0;
	for (n = 0; (n < c->i_count) && (c->i[n].i != i); n++);
	return (n == c->i_count) ? 0 : c->i[n].limit;
}

unsigned int
gpfs_cache_bag_get_limit (GPFsCache *c, GPFsBag *b)
{
	unsigned int n;

	if (!c || !b) return 0;
	for (n = 0; (n < c->b_count) && (c->b[n].b != b); n++);
	return (n == c->b_count) ? : c->b[n].limit;
}
