#include "config.h"
#include "gpfs-cache.h"

#include <stdlib.h>
#include <string.h>

typedef struct _GPFsCacheIf GPFsCacheIf;
struct _GPFsCacheIf {
	GPFsIf *i;

	/* Read */
	unsigned int l_read;
	GPFsIfFuncRead f_read; void *f_data_read;
	char *data;
	unsigned int size;

	/* Info */
	unsigned int l_info;
};

struct _GPFsCache {
	unsigned int ref_count;

	GPFsCacheIf *i;
	unsigned int i_count;
};

GPFsCache *
gpfs_cache_new (void)
{
	GPFsCache *c;

	c = malloc (sizeof (GPFsCache));
	if (!c)
		return NULL;
	memset (c, 0, sizeof (GPFsCache));
	c->ref_count = 1;

	return c;
}

void
gpfs_cache_ref (GPFsCache *c)
{
	if (c)
		c->ref_count++;
}

void
gpfs_cache_unref (GPFsCache *c)
{
	if (!c)
		return;
	if (!--c->ref_count) {

		/* Remove interfaces from cache */
		while (gpfs_cache_count_if (c))
			gpfs_cache_remove_if (c, c->i[0].i);

		free (c);
	}
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
	if (d->c->size + size > d->c->l_read) {
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
gpfs_cache_add_if (GPFsCache *c, GPFsIf *i)
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
	gpfs_if_ref (i);

	/* Intercept the communication */
	gpfs_if_get_func_read (i, &c->i[c->i_count - 1].f_read,
				  &c->i[c->i_count - 1].f_data_read);
	gpfs_if_set_func_read (i, f_read, c);
}

unsigned int
gpfs_cache_count_if (GPFsCache *c)
{
	return (c ? c->i_count : 0);
}

GPFsIf *
gpfs_cache_get_if (GPFsCache *c, unsigned int n)
{
	if (!c || (n >= c->i_count))
		return NULL;
	
	return c->i[n].i;
}

void
gpfs_cache_remove_if (GPFsCache *c, GPFsIf *i)
{
	unsigned int n;

	if (!c || !i)
		return;

	for (n = 0; (n < c->i_count) && (c->i[n].i != i); n++);
	if (n == c->i_count)
		return;
	gpfs_if_unref (c->i[n].i);
	free (c->i[n].data);
	memmove (&c->i[n], &c->i[n + 1], c->i_count - n - 1);
	c->i_count--;
}

void
gpfs_cache_if_set_limit_read (GPFsCache *c, GPFsIf *i, unsigned int l)
{
	unsigned int n;

	if (!c || !i)
		return;

	for (n = 0; (n < c->i_count) && (c->i[n].i != i); n++);
	if (n == c->i_count)
		return;
	c->i[n].l_read = l;
	if (c->i[n].size > l) {
		free (c->i[n].data);
		c->i[n].data = NULL;
		c->i[n].size = 0;
	}
}

void
gpfs_cache_if_set_limit_info (GPFsCache *c, GPFsIf *i, unsigned int l)
{
	unsigned int n;

	if (!c || !i)
		return;

	for (n = 0; (n < c->i_count) && (c->i[n].i != i); n++);
	if (n == c->i_count)
		return;
	c->i[n].l_info = l;
}

unsigned int
gpfs_cache_if_get_limit_info (GPFsCache *c, GPFsIf *i)
{
	unsigned int n;

	if (!c || !i)
		return 0;

	for (n = 0; (n < c->i_count) && (c->i[n].i != i); n++);
	return (n == c->i_count) ? 0 : c->i[n].l_info;
}

unsigned int
gpfs_cache_if_get_limit_read (GPFsCache *c, GPFsIf *i)
{
	unsigned int n;

	if (!c || !i)
		return 0;

	for (n = 0; (n < c->i_count) && (c->i[n].i != i); n++);
	return (n == c->i_count) ? 0 : c->i[n].l_read;
}
