#include "config.h"
#include "libgpfs/gpfs.h"
#include "libgpfs/gpfs-cache.h"
#include "libgpfs/gpfs-i18n.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*****************************************************************************/
/* The following functions are implemented by backends.                      */
/*****************************************************************************/

static unsigned int
func_count_files (GPFs *fs, GPFsErr *e, const char *folder, void *d)
{
	return 1;
}

static void
func_read1 (GPFsIf *i, GPFsErr *e, GPFsIfFuncReadCb f, void *f_data, void *d)
{
        const char data[] = "Hello world!";

	printf ("<<< Backend is reading data >>>\n");
        f (i, e, data, sizeof (data), f_data);
}

static void
func_read2 (GPFsIf *i, GPFsErr *e, GPFsIfFuncReadCb f, void *f_data, void *d)
{
	const char data[] = "Hello world!";
	unsigned int n;

	printf ("<<< Backend is reading data in steps of 3 bytes >>>\n");
	for (n = 0; n < sizeof (data); n += 3)
		f (i, e, data + n,
		   sizeof (data) - 3 >= n ? 3 : sizeof (data) - n, f_data);
}

static unsigned int
func_count_prop (GPFsIf *i, GPFsErr *e, void *ud)
{
	return 3;
}

static GPFsProp *
func_get_prop (GPFsIf *interface, GPFsErr *e, unsigned int n, void *ud)
{
	GPFsVal v;
	GPFsProp *i = NULL;

	switch (n) {
	case 0:
		gpfs_val_init (&v);
		v.t = GPFS_VAL_TYPE_UINT;
		v.v.v_uint = sizeof ("Hello world!");
		i = gpfs_prop_new ("size", _("Size"),
				_("The size of the file"), &v);
		gpfs_val_clear (&v);
		break;
	case 1:
		gpfs_val_init (&v);
		v.t = GPFS_VAL_TYPE_STRING;
		if (!strcmp (gpfs_if_get_name (interface), "default"))
			v.v.v_string = strdup (_("The default interface"));
		else
			v.v.v_string = strdup (_("The interface supplying a "
					         "preview"));
		i = gpfs_prop_new ("description", _("Description"),
				_("The description of the interface"), &v);
		gpfs_val_clear (&v);
		break;
	case 2:
		gpfs_val_init (&v);
		v.t = GPFS_VAL_TYPE_STRING;
		v.v.v_string = strdup ("image/png");
		i = gpfs_prop_new ("mime_type", _("Mime type"),
				_("The mime type of the file"), &v);
		gpfs_val_clear (&v);
		i->t = GPFS_ALT_TYPE_VALS;
		i->alt.vals.vals_count = 2;
		i->alt.vals.vals = malloc (sizeof (GPFsVal) *
					   i->alt.vals.vals_count);
		i->alt.vals.vals[0].t = GPFS_VAL_TYPE_STRING;
		i->alt.vals.vals[0].v.v_string = strdup ("image/png");
		i->alt.vals.vals[1].t = GPFS_VAL_TYPE_STRING;
		i->alt.vals.vals[1].v.v_string = strdup ("image/jpeg");
		break;
	default:
		gpfs_err_set (e, GPFS_ERR_TYPE_BAD_PARAMETERS,
			      _("Unknown property."));
	}

	return i;
}

static GPFsFile *
func_get_file (GPFs *fs, GPFsErr *e, const char *folder, unsigned int n, void *d)
{
	GPFsFile *f;
	GPFsIf *i;

	printf ("<<< Backend is setting up a file >>>\n");
	f = gpfs_file_new ();

	/* Interface 'default' */
	i = gpfs_if_new ("default");
	gpfs_if_set_func_read (i, func_read1, NULL);
	gpfs_if_set_func_count_prop (i, func_count_prop, NULL);
	gpfs_if_set_func_get_prop (i, func_get_prop, NULL);
	gpfs_file_add_if (f, i);
	gpfs_if_unref (i);

	/* Interface 'preview' */
	i = gpfs_if_new ("preview");
	gpfs_if_set_func_read (i, func_read2, NULL);
	gpfs_if_set_func_count_prop (i, func_count_prop, NULL);
	gpfs_if_set_func_get_prop (i, func_get_prop, NULL);
	gpfs_file_add_if (f, i);
	gpfs_if_unref (i);

	return f;
}

/*****************************************************************************/
/* Nearly everything below here is like what is being coded by frontends.    */
/*****************************************************************************/

static void
func_read_cb (GPFsIf *i, GPFsErr *e, const char *data, unsigned int size,
	      void *d)
{
	char *c;

	c = malloc (sizeof (char) * (size + 1));
	if (!c)
		return;
	c[size] = '\0';
	memcpy (c, data, size);
	printf ("Received data '%s' from interface '%s'!\n", c,
		gpfs_if_get_name (i));
	free ©;
}

int
main (int argc, char **argv)
{
	GPFs *fs;
	unsigned int n, j, k;
	GPFsFile *f;
	GPFsIf *i;
	GPFsCache *c;
	GPFsProp *prop;

	/* Create a cache */
	c = gpfs_cache_new ();

	/* Set up the filesystem */
	fs = gpfs_new ();
	gpfs_set_func_count_files (fs, func_count_files, NULL);
	gpfs_set_func_get_file    (fs, func_get_file, NULL);

	/* Test the filesystem */
	n = gpfs_count_files (fs, NULL, "/");
	printf ("Found %i file(s).\n", n);

	f = gpfs_get_file (fs, NULL, "/", 0);
	gpfs_unref (fs);

	n = gpfs_file_count_ifs (f);
	printf ("Found %i interface(s).\n", n);
	for (j = 0; j < n; j++) {
		i = gpfs_file_get_if (f, j);

		/* Add the interface to our cache. */
		gpfs_cache_add_if (c, i);
		gpfs_cache_if_set_limit_read (c, i, j * 20);

		printf (" %i: Interface '%s' with %i properties\n",
			j, gpfs_if_get_name (i), gpfs_if_count_prop (i, NULL));
		for (k = 0; k < gpfs_if_count_prop (i, NULL); k++) {
			prop = gpfs_if_get_prop (i, NULL, k);
			gpfs_prop_dump (prop);
		}
	}

	printf ("Testing interfaces...\n");
	for (j = 0; j < n; j++) {
		i = gpfs_file_get_if (f, j);
		gpfs_if_read (i, NULL, func_read_cb, NULL);
	}

	printf ("There are now %i interfaces in our cache.\n", 
		gpfs_cache_count_if ©);
	printf ("Testing interfaces again...\n");
	for (j = 0; j < n; j++) {
		i = gpfs_file_get_if (f, j);
		gpfs_if_read (i, NULL, func_read_cb, NULL);
	}

	gpfs_file_unref (f);

	gpfs_cache_unref ©;

	return 0;
}
