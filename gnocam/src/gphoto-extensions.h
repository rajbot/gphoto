
gboolean description_extract (gchar* description, guint* id, gchar** name, gchar** model, gchar** port, guint* speed);

Camera* gp_camera_new_by_description (gint id, gchar* name, gchar* model, gchar* port, gint speed);

void gp_camera_lock 	(Camera* camera);
void gp_camera_unlock 	(Camera* camera);


