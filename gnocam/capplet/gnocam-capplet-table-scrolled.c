#include "gnocam-capplet-table-scrolled.h"

#include <gal/util/e-util.h>
#include <gal/e-table/e-table.h>

#include "gnocam-capplet-table.h"

struct _GnoCamCappletTableScrolledPrivate
{
	ETable *table;
};

#define PARENT_TYPE E_TYPE_SCROLL_FRAME
static EScrollFrameClass *parent_class;

static void
gnocam_capplet_table_scrolled_destroy (GtkObject *object)
{
	GnoCamCappletTableScrolled *table;

	table = GNOCAM_CAPPLET_TABLE_SCROLLED (object);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gnocam_capplet_table_scrolled_finalize (GtkObject *object)
{
	GnoCamCappletTableScrolled *table;

	table = GNOCAM_CAPPLET_TABLE_SCROLLED (object);

	g_free (table->priv);

	GTK_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gnocam_capplet_table_scrolled_init (GnoCamCappletTableScrolled *table)
{
	table->priv = g_new0 (GnoCamCappletTableScrolledPrivate, 1);
}

static void
gnocam_capplet_table_scrolled_class_init (GnoCamCappletTableScrolledClass *klass)
{
	GtkObjectClass *object_class;

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy  = gnocam_capplet_table_scrolled_destroy;
	object_class->finalize = gnocam_capplet_table_scrolled_finalize;
}

GtkWidget *
gnocam_capplet_table_scrolled_new (CappletWidget *capplet)
{
	GnoCamCappletTableScrolled *table;

	table = GNOCAM_CAPPLET_TABLE_SCROLLED (gtk_widget_new (
			GNOCAM_TYPE_CAPPLET_TABLE_SCROLLED,
			"hadjustment", NULL, "vadjustment", NULL, NULL));
	e_scroll_frame_set_policy (E_SCROLL_FRAME (table), 
				   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	table->priv->table = gnocam_capplet_table_new (capplet);
	gtk_container_add (GTK_CONTAINER (table),
			   GTK_WIDGET (table->priv->table));
	gtk_widget_show (GTK_WIDGET (table->priv->table));

	return (GTK_WIDGET (table));
}

void
gnocam_capplet_table_scrolled_ok (GnoCamCappletTableScrolled *table)
{
	gnocam_capplet_table_ok (GNOCAM_CAPPLET_TABLE (table->priv->table));
}

void
gnocam_capplet_table_scrolled_revert (GnoCamCappletTableScrolled *table)
{
	gnocam_capplet_table_revert (GNOCAM_CAPPLET_TABLE (table->priv->table));
}

void
gnocam_capplet_table_scrolled_try (GnoCamCappletTableScrolled *table)
{
	gnocam_capplet_table_try (GNOCAM_CAPPLET_TABLE (table->priv->table));
}

void
gnocam_capplet_table_scrolled_cancel (GnoCamCappletTableScrolled *table)
{
	gnocam_capplet_table_cancel (GNOCAM_CAPPLET_TABLE (table->priv->table));
}

E_MAKE_TYPE (gnocam_capplet_table_scrolled, "GnoCamCappletTableScrolled", GnoCamCappletTableScrolled, gnocam_capplet_table_scrolled_class_init, gnocam_capplet_table_scrolled_init, PARENT_TYPE)

