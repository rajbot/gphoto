#include "config.h"
#include "gpfs-file.h"

#include <stdlib.h>
#include <string.h>

struct _GPFsFile
{
	unsigned int ref_count;

	GPFsIf **ifs;
	unsigned int count;
};

GPFsFile *
gpfs_file_new (void)
{
	GPFsFile *f;

	f = malloc (sizeof (GPFsFile));
	if (!f)
		return NULL;
	memset (f, 0, sizeof (GPFsFile));
	f->ref_count = 1;

	return f;
}

void
gpfs_file_ref (GPFsFile *f)
{
	if (f)
		f->ref_count++;
}

void
gpfs_file_unref (GPFsFile *f)
{
	unsigned int i;

	if (!f)
		return;
	if (!--f->ref_count) {
		for (i = 0; i < f->count; i++)
			gpfs_if_unref (f->ifs[i]);
		free (f->ifs);
		free (f);
	}
}

void
gpfs_file_add_if (GPFsFile *f, GPFsIf *i)
{
	GPFsIf **ifs;

	if (!f)
		return;
	
	ifs = realloc (f->ifs, sizeof (GPFsIf *) * (f->count + 1));
	if (!ifs)
		return;
	f->ifs = ifs;
	f->ifs[f->count] = i;
	gpfs_if_ref (i);
	f->count++;
}

unsigned int
gpfs_file_count_ifs (GPFsFile *f)
{
	return (f ? f->count : 0);
}

GPFsIf *
gpfs_file_get_if (GPFsFile *f, unsigned int n)
{
	if (!f || n >= f->count)
		return NULL;

	return f->ifs[n];
}
