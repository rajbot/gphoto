#include "config.h"
#include "gpfs.h"
#include "gpfs-i18n.h"
#include "gpfs-obj.h"

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
	GPFsObj parent;

	GPFsFuncCount     f_file_count  ; void *f_data_file_count  ;
	GPFsFuncGetFile   f_file_get    ; void *f_data_file_get    ;
	GPFsFuncCount     f_folder_count; void *f_data_folder_count;
	GPFsFuncGetFolder f_folder_get  ; void *f_data_folder_get  ;
};

GPFs *
gpfs_new (void)
{
	GPFsObj *o;

	o = gpfs_obj_new (sizeof (GPFs));
	if (!o) return NULL;

	return (GPFs *) o;
}

void
gpfs_set_func_file_count (GPFs *fs, GPFsFuncCount f, void *f_data)
{
	if (!fs) return;
	fs->f_file_count = f;
	fs->f_data_file_count = f_data;
}

void
gpfs_get_func_file_count (GPFs *fs, GPFsFuncCount *f, void **f_data)
{
	if (!fs) return;
	if (f) *f = fs->f_file_count;
	if (f_data) *f_data = fs->f_data_file_count;
}

void
gpfs_set_func_folder_count (GPFs *fs, GPFsFuncCount f, void *f_data)
{
	if (!fs) return;
	fs->f_folder_count = f;
	fs->f_data_folder_count = f_data;
}

void
gpfs_get_func_folder_count (GPFs *fs, GPFsFuncCount *f, void **f_data)
{
	if (!fs) return;
	if (f) *f = fs->f_folder_count;
	if (f_data) *f_data = fs->f_data_folder_count;
}

void
gpfs_set_func_file_get (GPFs *fs, GPFsFuncGetFile f, void *f_data)
{
	if (!fs) return;
	fs->f_file_get = f;
	fs->f_data_file_get = f_data;
}

void
gpfs_get_func_file_get (GPFs *fs, GPFsFuncGetFile *f, void **f_data)
{
	if (!fs) return;
	if (f) *f = fs->f_file_get;
	if (f_data) *f_data = fs->f_data_file_get;
}

void
gpfs_set_func_folder_get (GPFs *fs, GPFsFuncGetFolder f, void *f_data)
{
	if (!fs) return;
	fs->f_folder_get = f;
	fs->f_data_folder_get = f_data;
}

void
gpfs_get_func_folder_get (GPFs *fs, GPFsFuncGetFolder *f, void **f_data)
{
	if (!fs) return;
	if (f) *f = fs->f_folder_get;
	if (f_data) *f_data = fs->f_data_folder_get;
}

unsigned int
gpfs_file_count (GPFs *fs, GPFsErr *e, const char *folder)
{
	CN0 (fs, e);

	if (!fs->f_file_count) {
		gpfs_err_set (e, GPFS_ERR_TYPE_NOT_SUPPORTED,
			_("This filesystem doesn't support counting files."));
		return 0;
	}

	return fs->f_file_count (fs, e, folder, fs->f_data_file_count);
}

unsigned int
gpfs_folder_count (GPFs *fs, GPFsErr *e, const char *folder)
{
	CN0 (fs, e);

	if (!fs->f_folder_count) {
		gpfs_err_set (e, GPFS_ERR_TYPE_NOT_SUPPORTED,
			_("This filesystem doesn't support counting folders."));
		return 0;
	}

	return fs->f_folder_count (fs, e, folder, fs->f_data_folder_count);
}
		
GPFsFile *
gpfs_file_get (GPFs *fs, GPFsErr *e, const char *folder, unsigned int n)
{
	CNN (fs, e);

	if (!fs->f_file_get) {
		gpfs_err_set (e, GPFS_ERR_TYPE_NOT_SUPPORTED,
			_("This filesystem doesn't support getting files."));
		return 0;
	}

	return fs->f_file_get (fs, e, folder, n, fs->f_data_file_get);
}

char *
gpfs_folder_get (GPFs *fs, GPFsErr *e, const char *folder, unsigned int n)
{
	CNN (fs, e);

	if (!fs->f_folder_get) {
		gpfs_err_set (e, GPFS_ERR_TYPE_NOT_SUPPORTED,
			_("This filesystem doesn't support getting folders."));
		return 0;
	}

	return fs->f_folder_get (fs, e, folder, n, fs->f_data_folder_get);
}
