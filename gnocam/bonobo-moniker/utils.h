
/**************/
/* Prototypes */
/**************/

Camera* util_camera_new (gchar* name, CORBA_Environment* ev);


void menu_prepare 	(CameraWidget* widget, xmlNodePtr popup, xmlNodePtr command, xmlNsPtr ns);
void menu_fill		(GnoCamControl* control, gchar* path, CameraWidget* window, CameraWidget* widget, gboolean camera);



