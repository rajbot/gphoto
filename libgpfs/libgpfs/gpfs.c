#include "config.h"
#include "gpfs.h"
#include "gpfs-i18n.h"

#include <stdlib.h>
#include <string.h>

#define CNV(p,e) {                                                      \
        if (!p) {                                                       \
                gpfs_err_set (e, GPFS_ERR_TYPE_BAD_PARAMETERS,          \
                            _("You need to supply a filesystem."));     \
                return;                                                 \
        }                                                               \
}

#define CN0(p,e) {                                                      \
        if (!p) {                                                       \
                gpfs_err_set (e, GPFS_ERR_TYPE_BAD_PARAMETERS,          \
                            _("You need to supply a filesystem."));     \
                return 0;                                               \
        }                                                               \
}

#define CNN(p,e) {							\
	if (!p) {							\
		gpfs_err_set (e, GPFS_ERR_TYPE_BAD_PARAMETERS,		\
			    _("You need to supply a filesystem."));	\
		return NULL;						\
	}								\
}

struct _GPFs
{
	unsigned int ref_count;

	GPFsFuncCountFiles   f_count_files  ; void *f_data_count_files  ;
	GPFsFuncGetFile      f_get_file     ; void *f_data_get_file     ;
	GPFsFuncCountFolders f_count_folders; void *f_data_count_folders;
	GPFsFuncGetFolder    f_get_folder   ; void *f_data_get_folder   ;
};

GPFs *
gpfs_new (void)
{
	GPFs *fs;

	fs = malloc (sizeof (GPFs));
	if (!fs)
		return NULL;
	memset (fs, 0, sizeof (GPFs));
	fs->ref_count = 1;

	return (fs);
}

void
gpfs_ref (GPFs *fs)
{
	if (fs)
		fs->ref_count++;
}

void
gpfs_unref (GPFs *fs)
{
	if (!fs)
		return;
	if (!--fs->ref_count)
		free (fs);
}

void
gpfs_set_func_count_files (GPFs *fs, GPFsFuncCountFiles f, void *f_data)
{
	if (!fs) return;
	fs->f_count_files = f;
	fs->f_data_count_files = f_data;
}

void
gpfs_get_func_count_files (GPFs *fs, GPFsFuncCountFiles *f, void **f_data)
{
	if (!fs) return;
	if (f) *f = fs->f_count_files;
	if (f_data) *f_data = fs->f_data_count_files;
}

void
gpfs_set_func_count_folders (GPFs *fs, GPFsFuncCountFolders f, void *f_data)
{
	if (!fs) return;
	fs->f_count_folders = f;
	fs->f_data_count_folders = f_data;
}

void
gpfs_get_func_count_folders (GPFs *fs, GPFsFuncCountFolders *f, void **f_data)
{
	if (!fs) return;
	if (f) *f = fs->f_count_folders;
	if (f_data) *f_data = fs->f_data_count_folders;
}

void
gpfs_set_func_get_file (GPFs *fs, GPFsFuncGetFile f, void *f_data)
{
	if (!fs) return;
	fs->f_get_file = f;
	fs->f_data_get_file = f_data;
}

void
gpfs_get_func_get_file (GPFs *fs, GPFsFuncGetFile *f, void **f_data)
{
	if (!fs) return;
	if (f) *f = fs->f_get_file;
	if (f_data) *f_data = fs->f_data_get_file;
}

void
gpfs_set_func_get_folder (GPFs *fs, GPFsFuncGetFolder f, void *f_data)
{
	if (!fs) return;
	fs->f_get_folder = f;
	fs->f_data_get_folder = f_data;
}

void
gpfs_get_func_get_folder (GPFs *fs, GPFsFuncGetFolder *f, void **f_data)
{
	if (!fs) return;
	if (f) *f = fs->f_get_folder;
	if (f_data) *f_data = fs->f_data_get_folder;
}

unsigned int
gpfs_count_files (GPFs *fs, GPFsErr *e, const char *folder)
{
	CN0 (fs, e);

	if (!fs->f_count_files) {
		gpfs_err_set (e, GPFS_ERR_TYPE_NOT_SUPPORTED,
			_("This filesystem doesn't support counting files."));
		return 0;
	}

	return fs->f_count_files (fs, e, folder, fs->f_data_count_files);
}

unsigned int
gpfs_count_folders (GPFs *fs, GPFsErr *e, const char *folder)
{
	CN0 (fs, e);

	if (!fs->f_count_folders) {
		gpfs_err_set (e, GPFS_ERR_TYPE_NOT_SUPPORTED,
			_("This filesystem doesn't support counting folders."));
		return 0;
	}

	return fs->f_count_folders (fs, e, folder, fs->f_data_count_folders);
}
		
GPFsFile *
gpfs_get_file (GPFs *fs, GPFsErr *e, const char *folder, unsigned int n)
{
	CNN (fs, e);

	if (!fs->f_get_file) {
		gpfs_err_set (e, GPFS_ERR_TYPE_NOT_SUPPORTED,
			_("This filesystem doesn't support getting files."));
		return 0;
	}

	return fs->f_get_file (fs, e, folder, n, fs->f_data_get_file);
}

char *
gpfs_get_folder (GPFs *fs, GPFsErr *e, const char *folder, unsigned int n)
{
	CNN (fs, e);

	if (!fs->f_get_folder) {
		gpfs_err_set (e, GPFS_ERR_TYPE_NOT_SUPPORTED,
			_("This filesystem doesn't support getting folders."));
		return 0;
	}

	return fs->f_get_folder (fs, e, folder, n, fs->f_data_get_folder);
}
