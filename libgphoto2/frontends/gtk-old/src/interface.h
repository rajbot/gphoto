/*
 * DO NOT EDIT THIS FILE - it is generated by Glade.
 */

GtkWidget* create_main_window (void);
GtkWidget* create_confirm_window (void);
GtkWidget* create_progress_window (void);
GtkWidget* create_message_window (void);
GtkWidget* create_message_window_long (void);
GtkWidget* create_message_window_transient (void);
GtkWidget* create_select_camera_window (void);

int gp_interface_message(char *message);
int gp_interface_message_long(char *message);
int gp_interface_status(char *message);
int gp_interface_progress(float percentage);
int gp_interface_confirm(char *message);
