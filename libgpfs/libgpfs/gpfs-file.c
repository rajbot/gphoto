#include "config.h"
#include "gpfs-file.h"

#include <stdlib.h>
#include <string.h>

#include "gpfs-obj.h"

struct _GPFsFile
{
	GPFsObj parent;

	GPFsIf **ifs;
	unsigned int count;
};

static void
gpfs_file_free (GPFsObj *o)
{
	GPFsFile *f = (GPFsFile *) o;
	unsigned int i;

	for (i = 0; i < f->count; i++)
		gpfs_obj_unref (GPFS_OBJ (f->ifs[i]));
	free (f->ifs);
}

GPFsFile *
gpfs_file_new (void)
{
	GPFsFile *f;

	f = malloc (sizeof (GPFsFile));
	if (!f)
		return NULL;
	memset (f, 0, sizeof (GPFsFile));
	gpfs_obj_init (GPFS_OBJ (f));
	((GPFsObj *) f)->f_free = gpfs_file_free;

	return f;
}

void
gpfs_file_if_add (GPFsFile *f, GPFsIf *i)
{
	GPFsIf **ifs;

	if (!f)
		return;
	
	ifs = realloc (f->ifs, sizeof (GPFsIf *) * (f->count + 1));
	if (!ifs)
		return;
	f->ifs = ifs;
	f->ifs[f->count] = i;
	gpfs_obj_ref (GPFS_OBJ (i));
	f->count++;
}

unsigned int
gpfs_file_if_count (GPFsFile *f)
{
	return (f ? f->count : 0);
}

GPFsIf *
gpfs_file_if_get (GPFsFile *f, unsigned int n)
{
	if (!f || n >= f->count)
		return NULL;

	return f->ifs[n];
}
