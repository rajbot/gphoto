
/**************/
/* Prototypes */
/**************/

void capture_image 	(Camera* camera);
void capture_video 	(Camera* camera);

void properties 	(Camera* camera);

void gp_widget_to_xml 	(BonoboUIComponent* component, CameraWidget* window, CameraWidget* widget, xmlNodePtr popup, xmlNodePtr command, xmlNsPtr ns);

void ui_set_values_from_widget (BonoboUIComponent* component, CameraWidget* window, CameraWidget* widget);

