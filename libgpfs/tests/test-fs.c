#include "config.h"
#include "libgpfs/gpfs.h"
#include "libgpfs/gpfs-cache.h"
#include "libgpfs/gpfs-i18n.h"
#include "libgpfs/gpfs-obj.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*****************************************************************************/
/* The following functions are implemented by backends.                      */
/*****************************************************************************/

static unsigned int
func_file_count (GPFs *fs, GPFsErr *e, const char *folder, void *d)
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
func_prop_count (GPFsBag *b, GPFsErr *e, void *ud)
{
	return 3;
}

static GPFsProp *
func_prop_get (GPFsBag *b, GPFsErr *e, unsigned int n, void *ud)
{
	GPFsVal v;
	GPFsProp *i = NULL;

	switch (n) {
	case 0:
		gpfs_val_init (&v);
		v.t = GPFS_VAL_TYPE_UINT;
		v.v.v_uint = sizeof ("Hello world!");
		i = gpfs_prop_new (0, _("Size"),
				_("The size of the file"), v);
		gpfs_val_clear (&v);
		break;
	case 1:
		gpfs_val_init (&v);
		v.t = GPFS_VAL_TYPE_STRING;
		v.v.v_string = strdup (_("Some interface"));
		i = gpfs_prop_new (1, _("Description"),
				_("The description of the interface"), v);
		gpfs_val_clear (&v);
		break;
	case 2:
		gpfs_val_init (&v);
		v.t = GPFS_VAL_TYPE_STRING;
		v.v.v_string = strdup ("image/png");
		i = gpfs_prop_new (2, _("Mime type"),
				_("The mime type of the file"), v);
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

static unsigned int
func_bag_count (GPFsObj *o, GPFsErr *e, void *ud)
{
	return 1;
}

static GPFsBag *
func_bag_get (GPFsObj *o, GPFsErr *e, unsigned int n, void *ud)
{
	GPFsBag *b;

	b = gpfs_bag_new ();
	gpfs_bag_set_func_prop_count (b, func_prop_count, NULL);
	gpfs_bag_set_func_prop_get (b, func_prop_get, NULL);

	return b;
}

static GPFsFile *
func_file_get (GPFs *fs, GPFsErr *e, const char *folder, unsigned int n, void *d)
{
	GPFsFile *f;
	GPFsIf *i;

	printf ("<<< Backend is creating a file >>>\n");
	f = gpfs_file_new ();

	/* Interface 'default' */
	printf ("<<< Backend is adding an interface >>>\n");
	i = gpfs_if_new ();
	gpfs_obj_set_name (GPFS_OBJ (i), NULL, "default");
	gpfs_if_set_func_read (i, func_read1, NULL);
	gpfs_obj_set_func_bag_count (GPFS_OBJ (i), func_bag_count, NULL);
	gpfs_obj_set_func_bag_get (GPFS_OBJ (i), func_bag_get, NULL);
	gpfs_file_if_add (f, i);
	gpfs_obj_unref (GPFS_OBJ (i));

	/* Interface 'preview' */
	printf ("<<< Backend is adding an interface >>>\n");
	i = gpfs_if_new ();
	gpfs_obj_set_name (GPFS_OBJ (i), NULL, "preview");
	gpfs_if_set_func_read (i, func_read2, NULL);
	gpfs_obj_set_func_bag_count (GPFS_OBJ (i), func_bag_count, NULL);
	gpfs_obj_set_func_bag_get (GPFS_OBJ (i), func_bag_get, NULL);
	gpfs_file_if_add (f, i);
	gpfs_obj_unref (GPFS_OBJ (i));

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
		gpfs_obj_get_name (GPFS_OBJ (i), NULL));
	free (c);
}

int
main (int argc, char **argv)
{
	GPFs *fs;
	unsigned int n, j;
	GPFsFile *f;
	GPFsIf *i;
	GPFsCache *c;

	/* Create a cache */
	c = gpfs_cache_new ();

	/* Set up the filesystem */
	fs = gpfs_new ();
	gpfs_set_func_file_count (fs, func_file_count, NULL);
	gpfs_set_func_file_get    (fs, func_file_get, NULL);

	/* Test the filesystem */
	printf ("Counting files...\n");
	n = gpfs_file_count (fs, NULL, "/");
	printf ("Found %i file(s).\n", n);

	printf ("Getting first file...\n");
	f = gpfs_file_get (fs, NULL, "/", 0);
	printf ("... done.\n");

	gpfs_obj_unref (GPFS_OBJ (fs));

	printf ("Counting interfaces...\n");
	n = gpfs_file_if_count (f);
	printf ("... done. Found %i interface(s).\n", n);

	for (j = 0; j < n; j++) {
		i = gpfs_file_if_get (f, j);

		/* Add the interface to our cache. */
		gpfs_cache_if_add (c, i);
		gpfs_cache_if_set_limit (c, i, j * 20);

#if 0
		printf (" %i: Interface '%s' with %i properties\n",
			j, gpfs_if_get_name (i), gpfs_if_count_prop (i, NULL));
		for (k = 0; k < gpfs_if_count_prop (i, NULL); k++) {
			prop = gpfs_if_get_prop (i, NULL, k);
			gpfs_prop_dump (prop);
		}
#endif
	}

	printf ("Testing interfaces...\n");
	for (j = 0; j < n; j++) {
		i = gpfs_file_if_get (f, j);
		gpfs_if_read (i, NULL, func_read_cb, NULL);
	}

	printf ("There are now %i interfaces in our cache.\n", 
		gpfs_cache_if_count (c));

	printf ("Testing interfaces again...\n");
	for (j = 0; j < n; j++) {
		i = gpfs_file_if_get (f, j);
		gpfs_if_read (i, NULL, func_read_cb, NULL);
	}
	printf ("... done.\n");

	printf ("Releasing file...\n");
	gpfs_obj_unref (GPFS_OBJ (f));
	printf ("... done.\n");

	printf ("Releasing cache...\n");
	gpfs_obj_unref (GPFS_OBJ (c));
	printf ("... done.\n");

	return 0;
}
