
/**************/
/* Prototypes */
/**************/

void capture_image 	(Camera* camera);
void capture_video 	(Camera* camera);

void popup_prepare 	(BonoboUIComponent* component, CameraWidget* widget, xmlNodePtr popup, xmlNodePtr command, xmlNsPtr ns);
void popup_fill		(BonoboUIComponent* component, gchar* path, CameraWidget* window, CameraWidget* widget, gboolean camera);



