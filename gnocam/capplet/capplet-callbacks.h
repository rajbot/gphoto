
void camera_help (void);
void camera_apply (void);
void camera_ok (void);
void camera_try (void);
void camera_revert (void);
void camera_cancel (void);

void on_button_camera_add_clicked (GtkButton *button, gpointer user_data);
void on_button_camera_update_clicked (GtkButton *button, gpointer user_data);
void on_button_camera_delete_clicked (GtkButton *button, gpointer user_data);
void on_button_camera_properties_clicked (GtkButton *button, gpointer user_data);

void on_clist_row_selection_changed (GtkWidget *clist, gint row, gint column, GdkEventButton *event, gpointer user_data);

