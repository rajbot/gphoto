
Camera* gp_camera_new_by_description (GladeXML* xml, gchar* description);

gboolean gp_camera_update_by_description (Camera** camera, gchar* description);

CameraWidget* gp_widget_clone (CameraWidget* widget);

