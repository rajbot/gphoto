#ifndef __GPFS_INFO_H__
#define __GPFS_INFO_H__

#include <libgpfs/gpfs-val.h>
#include <libgpfs/gpfs-err.h>

typedef enum {
	GPFS_ALT_TYPE_NONE,
	GPFS_ALT_TYPE_VALS,
	GPFS_ALT_TYPE_RANGE
} GPFsAltType;

typedef struct _GPFsInfo     GPFsInfo;
typedef struct _GPFsInfoPriv GPFsInfoPriv;

struct _GPFsInfo {
	GPFsInfoPriv *priv;

	/* Alternatives */
	GPFsAltType t;
	union {

		/* One or more values */
		struct {
			GPFsVal     *vals;
			unsigned int vals_count;
		} vals;

		/* A range of values */
		struct {
			unsigned int precision;
			float min, max, incr;
		} range;
	} alt;
};

GPFsInfo *gpfs_info_new   (const char *id, const char *name,
			   const char *description, GPFsVal *);
void      gpfs_info_ref   (GPFsInfo *);
void      gpfs_info_unref (GPFsInfo *);

const char *gpfs_info_get_id          (GPFsInfo *);
const char *gpfs_info_get_name        (GPFsInfo *);
const char *gpfs_info_get_description (GPFsInfo *);

void      gpfs_info_get_val (GPFsInfo *, GPFsErr *, GPFsVal *);
void      gpfs_info_set_val (GPFsInfo *, GPFsErr *, GPFsVal *);

typedef void (* GPFsInfoFuncSetVal) (GPFsInfo *, GPFsErr *, GPFsVal *, void *);
void gpfs_info_set_func_set_val (GPFsInfo *, GPFsInfoFuncSetVal  , void * );
void gpfs_info_get_func_set_val (GPFsInfo *, GPFsInfoFuncSetVal *, void **);

void gpfs_info_dump  (GPFsInfo *);

#endif
