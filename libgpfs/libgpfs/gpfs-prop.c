#include <config.h>
#include "gpfs-prop.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gpfs-i18n.h"

#define CNN(i,e) {                                                      \
        if (!i) {                                                       \
                gpfs_err_set (e, GPFS_ERR_TYPE_BAD_PARAMETERS,          \
                        _("You need to supply a piece of "		\
			  "proprmation."));         			\
                return NULL;                                            \
        }                                                               \
}

#define CNV(i,e) {                                                      \
        if (!i) {                                                       \
                gpfs_err_set (e, GPFS_ERR_TYPE_BAD_PARAMETERS,          \
                            _("You need to supply a piece of "		\
			      "proprmation."));     			\
                return;                                                 \
        }                                                               \
}

struct _GPFsPropPriv 
{
	unsigned int ref_count;
	unsigned char *id, *name, *description;

	GPFsPropFuncSetVal f_set_val; void *f_data_set_val;

	GPFsVal val;
};

GPFsProp *
gpfs_prop_new (const char *id, const char *name, const char *description,
	       GPFsVal *v)
{
	GPFsProp *i;

	i = malloc (sizeof (GPFsProp));
	if (!i) return NULL;
	memset (i, 0, sizeof (GPFsProp));
	i->priv = malloc (sizeof (GPFsPropPriv));
	if (!i->priv) {
		free (i);
		return NULL;
	}
	memset (i->priv, 0, sizeof (GPFsPropPriv));
	i->priv->ref_count = 1;

	i->priv->id          = strdup (id);
	i->priv->name        = strdup (name);
	i->priv->description = strdup (description);
	gpfs_val_copy (&i->priv->val, v);

	return i;
}

void
gpfs_prop_ref (GPFsProp *i)
{
	if (i) i->priv->ref_count++;
}

void
gpfs_prop_unref (GPFsProp *i)
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
gpfs_prop_get_description (GPFsProp *i)
{
	return i ? i->priv->description : NULL;
}

const char *
gpfs_prop_get_name (GPFsProp *i)
{
	return i ? i->priv->name : NULL;
}

const char *
gpfs_prop_get_id (GPFsProp *i)
{
	return i ? i->priv->name : NULL;
}

void
gpfs_prop_get_val (GPFsProp *i, GPFsErr *e, GPFsVal *v)
{
	CNV(i,e);

	gpfs_val_copy (v, &i->priv->val);
}

void
gpfs_prop_set_val (GPFsProp *i, GPFsErr *e, GPFsVal *v)
{
	CNV(i,e);

	if (!i->priv->f_set_val) {
		gpfs_err_set (e, GPFS_ERR_TYPE_NOT_SUPPORTED,
			      _("This value of this piece of proprmation "
				"cannot be changed."));
		return;
	}

	i->priv->f_set_val (i, e, v, i->priv->f_data_set_val);
}

void
gpfs_prop_set_func_set_val (GPFsProp *i, GPFsPropFuncSetVal f, void *f_data)
{
	if (!i) return;
	i->priv->f_set_val = f;
	i->priv->f_data_set_val = f_data;
}

void
gpfs_prop_get_func_set_val (GPFsProp *i, GPFsPropFuncSetVal *f, void **f_data)
{
	if (!i) return;
	if (f) *f = i->priv->f_set_val;
	if (f_data) *f_data = i->priv->f_data_set_val;
}

void
gpfs_prop_dump (GPFsProp *i)
{
	GPFsVal v;
	unsigned int n;

	printf ("Proprmation '%s':\n", gpfs_prop_get_id (i));
	printf (" - Name: '%s'\n", gpfs_prop_get_name (i));
	printf (" - Description: '%s'\n", gpfs_prop_get_description (i));
	gpfs_val_init (&v);
	gpfs_prop_get_val (i, NULL, &v);
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
