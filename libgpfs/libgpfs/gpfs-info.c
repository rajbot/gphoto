#include <config.h>
#include "gpfs-info.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gpfs-i18n.h"

#define CNN(i,e) {                                                      \
        if (!i) {                                                       \
                gpfs_err_set (e, GPFS_ERR_TYPE_BAD_PARAMETERS,          \
                        _("You need to supply a piece of "		\
			  "information."));         			\
                return NULL;                                            \
        }                                                               \
}

#define CNV(i,e) {                                                      \
        if (!i) {                                                       \
                gpfs_err_set (e, GPFS_ERR_TYPE_BAD_PARAMETERS,          \
                            _("You need to supply a piece of "		\
			      "information."));     			\
                return;                                                 \
        }                                                               \
}

struct _GPFsInfoPriv 
{
	unsigned int ref_count;
	unsigned char *id, *name, *description;

	GPFsInfoFuncSetVal f_set_val; void *f_data_set_val;

	GPFsVal val;
};

GPFsInfo *
gpfs_info_new (const char *id, const char *name, const char *description,
	       GPFsVal *v)
{
	GPFsInfo *i;

	i = malloc (sizeof (GPFsInfo));
	if (!i) return NULL;
	memset (i, 0, sizeof (GPFsInfo));
	i->priv = malloc (sizeof (GPFsInfoPriv));
	if (!i->priv) {
		free (i);
		return NULL;
	}
	memset (i->priv, 0, sizeof (GPFsInfoPriv));
	i->priv->ref_count = 1;

	i->priv->id          = strdup (id);
	i->priv->name        = strdup (name);
	i->priv->description = strdup (description);
	gpfs_val_copy (&i->priv->val, v);

	return i;
}

void
gpfs_info_ref (GPFsInfo *i)
{
	if (i) i->priv->ref_count++;
}

void
gpfs_info_unref (GPFsInfo *i)
{
	unsigned int n;

	if (!i) return;
	if (!--i->priv->ref_count) {
		free (i->priv->id);
		free (i->priv->name);
		free (i->priv->description);
		gpfs_val_clear (&i->priv->val);
		free (i->priv);
		switch (i->t) {
		case GPFS_ALT_TYPE_VALS:
			for (n = 0; n < i->alt.vals.vals_count; n++)
				gpfs_val_clear (&i->alt.vals.vals[n]);
			free (i->alt.vals.vals);
			break;
		default:
			break;
		}
		free (i);
	}
}

const char *
gpfs_info_get_description (GPFsInfo *i)
{
	return i ? i->priv->description : NULL;
}

const char *
gpfs_info_get_name (GPFsInfo *i)
{
	return i ? i->priv->name : NULL;
}

const char *
gpfs_info_get_id (GPFsInfo *i)
{
	return i ? i->priv->name : NULL;
}

void
gpfs_info_get_val (GPFsInfo *i, GPFsErr *e, GPFsVal *v)
{
	CNV(i,e);

	gpfs_val_copy (v, &i->priv->val);
}

void
gpfs_info_set_val (GPFsInfo *i, GPFsErr *e, GPFsVal *v)
{
	CNV(i,e);

	if (!i->priv->f_set_val) {
		gpfs_err_set (e, GPFS_ERR_TYPE_NOT_SUPPORTED,
			      _("This value of this piece of information "
				"cannot be changed."));
		return;
	}

	i->priv->f_set_val (i, e, v, i->priv->f_data_set_val);
}

void
gpfs_info_set_func_set_val (GPFsInfo *i, GPFsInfoFuncSetVal f, void *f_data)
{
	if (!i) return;
	i->priv->f_set_val = f;
	i->priv->f_data_set_val = f_data;
}

void
gpfs_info_get_func_set_val (GPFsInfo *i, GPFsInfoFuncSetVal *f, void **f_data)
{
	if (!i) return;
	if (f) *f = i->priv->f_set_val;
	if (f_data) *f_data = i->priv->f_data_set_val;
}

void
gpfs_info_dump (GPFsInfo *i)
{
	GPFsVal v;
	unsigned int n;

	printf ("Information '%s':\n", gpfs_info_get_id (i));
	printf (" - Name: '%s'\n", gpfs_info_get_name (i));
	printf (" - Description: '%s'\n", gpfs_info_get_description (i));
	gpfs_val_init (&v);
	gpfs_info_get_val (i, NULL, &v);
	printf (" - Type: '%s'\n", gpfs_val_type_get_name (v.t));
	switch (v.t) {
	case GPFS_VAL_TYPE_STRING:
		printf (" - Value: '%s'\n", v.v.v_string);
		break;
	case GPFS_VAL_TYPE_UINT:
		printf (" - Value: %i\n", v.v.v_uint);
		break;
	case GPFS_VAL_TYPE_INT:
		printf (" - Value: %i\n", v.v.v_int);
		break;
	default:
		printf (" - Value unknown\n");
	}
	gpfs_val_clear (&v);

	/* Print alternatives */
	switch (i->t) {
	case GPFS_ALT_TYPE_VALS:
		if (i->alt.vals.vals_count)
		  printf (" - Alternatives:\n");
		else
		  printf (" - No alternatives\n");
		for (n = 0; n < i->alt.vals.vals_count; n++)
		  switch (i->alt.vals.vals[n].t) {
		  case GPFS_VAL_TYPE_STRING:
		    printf ("    * '%s'\n", i->alt.vals.vals[n].v.v_string);
		    break;
		  case GPFS_VAL_TYPE_INT:
		    printf ("    * %i\n", i->alt.vals.vals[n].v.v_int);
		    break;
		  case GPFS_VAL_TYPE_UINT:
		    printf ("    * %i\n", i->alt.vals.vals[n].v.v_uint);
		    break;
		  default:
		    printf (" - Value unknown\n");
		  }
		break;
	case GPFS_ALT_TYPE_RANGE:
		printf (" - Alternative values in range between %f and %f "
			"(precision %i, step %f)\n", i->alt.range.min,
			i->alt.range.max, i->alt.range.precision,
			i->alt.range.incr);
		break;
	default:
		break;
	}
}
