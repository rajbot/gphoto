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
func_count_info (GPFsIf *i, GPFsErr *e, void *ud)
{
	return 2;
}

static void
func_get_info (GPFsIf *i, GPFsErr *e, unsigned int n, GPFsInfo *info, void *ud)
{
	gpfs_info_init (info);
	switch (n) {
	case 0:
		info->id = strdup ("size");
		info->name = strdup (_("Size"));
		info->description = strdup (_("The size of the file"));
		info->t = GPFS_INFO_TYPE_UINT;
		info->v.v_uint = sizeof ("Hello world!");
		break;
	case 1:
		info->id = strdup ("description");
		info->name = strdup (_("Description"));
		info->description = strdup (_("The description of the "
					      "interface"));
		info->t = GPFS_INFO_TYPE_STRING;
		if (!strcmp (gpfs_if_get_name (i), "default"))
			info->v.v_string = strdup (_("The default interface"));
		else
			info->v.v_string = strdup (_("The interface supplying "
						     "a preview"));
		break;
	default:
		gpfs_err_set (e, GPFS_ERR_TYPE_BAD_PARAMETERS,
			      _("Unknown piece of information."));
	}
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
	gpfs_if_set_func_count_info (i, func_count_info, NULL);
	gpfs_if_set_func_get_info (i, func_get_info, NULL);
	gpfs_file_add_if (f, i);
	gpfs_if_unref (i);

	/* Interface 'preview' */
	i = gpfs_if_new ("preview");
	gpfs_if_set_func_read (i, func_read2, NULL);
	gpfs_if_set_func_count_info (i, func_count_info, NULL);
	gpfs_if_set_func_get_info (i, func_get_info, NULL);
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
	free (c);
}

int
main (int argc, char **argv)
{
	GPFs *fs;
	unsigned int n, j, k;
	GPFsFile *f;
	GPFsIf *i;
	GPFsCache *c;
	GPFsInfo info;

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

		printf (" %i: Interface '%s' with %i piece(s) of information\n",
			j, gpfs_if_get_name (i), gpfs_if_count_info (i, NULL));
		for (k = 0; k < gpfs_if_count_info (i, NULL); k++) {
			gpfs_info_init (&info);
			gpfs_if_get_info (i, NULL, k, &info);
			printf ("     * '%s' (type '%s'): '%s'\n", info.name,
				gpfs_info_type_get_name (info.t),
				info.description);
			switch (info.t) {
			case GPFS_INFO_TYPE_STRING:
				printf ("       Value: '%s'\n",
					info.v.v_string);
				break;
			case GPFS_INFO_TYPE_INT:
				printf ("       Value: %i\n", info.v.v_int);
				break;
			case GPFS_INFO_TYPE_UINT:
				printf ("       Value: %i\n", info.v.v_uint);
				break;
			}
			gpfs_info_clear (&info);
		}
	}

	printf ("Testing interfaces...\n");
	for (j = 0; j < n; j++) {
		i = gpfs_file_get_if (f, j);
		gpfs_if_read (i, NULL, func_read_cb, NULL);
	}

	printf ("There are now %i interfaces in our cache.\n", 
		gpfs_cache_count_if (c));
	printf ("Testing interfaces again...\n");
	for (j = 0; j < n; j++) {
		i = gpfs_file_get_if (f, j);
		gpfs_if_read (i, NULL, func_read_cb, NULL);
	}

	gpfs_file_unref (f);

	gpfs_cache_unref (c);

	return 0;
}
