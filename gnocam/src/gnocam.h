
/********************/
/* Type Definitions */
/********************/

typedef struct {
	guint		id;
	gchar* 		name;
} frontend_data_t;

typedef enum {
	GNOCAM_VIEW_MODE_NONE,
	GNOCAM_VIEW_MODE_PREVIEW,
	GNOCAM_VIEW_MODE_FILE
} GnoCamViewMode;

/**************/
/* Prototypes */
/**************/

void on_about_activate           (BonoboUIComponent* component, gpointer user_data, const gchar* path);



