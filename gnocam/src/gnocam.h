
/********************/
/* Type Definitions */
/********************/

typedef struct {
	guint		id;
	gchar* 		name;
	GtkTreeItem*	item;
} frontend_data_t;

typedef enum {
	GNOCAM_VIEW_MODE_NONE,
	GNOCAM_VIEW_MODE_PREVIEW,
	GNOCAM_VIEW_MODE_FILE
} GnoCamViewMode;

/**************/
/* Prototypes */
/**************/

void on_exit_activate            (GtkWidget* widget, gpointer user_data);
void on_preferences_activate     (GtkWidget* widget, gpointer user_data);
void on_gnocam_manual_activate   (GtkWidget* widget, gpointer user_data);
void on_about_activate           (GtkWidget* widget, gpointer user_data);



