
/* Type Definitions */

typedef struct {
	guint		id;
	gchar* 		name;
	guint		ref_count;
	GladeXML*	xml;
	GladeXML*	xml_properties;
	GladeXML*	xml_preview;
} frontend_data_t;

/* Prototypes */

int gp_frontend_status 		(Camera* camera, char *status);
int gp_frontend_progress 	(Camera* camera, CameraFile *file, float percentage);
int gp_frontend_message 	(Camera* camera, char *message);
int gp_frontend_confirm 	(Camera* camera, char *message);
int gp_frontend_prompt 		(Camera* camera, CameraWidget *window);

