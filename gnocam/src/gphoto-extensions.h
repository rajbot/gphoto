
gboolean description_extract (gchar* description, guint* id, gchar** name, gchar** model, gchar** port, guint* speed);

Camera* gp_camera_new_by_description (gchar* description);

void gp_camera_ref 	(Camera* camera);
void gp_camera_unref 	(Camera* camera);


