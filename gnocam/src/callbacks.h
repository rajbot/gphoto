
gboolean on_tree_item_camera_button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
gboolean on_tree_item_file_button_press_event   (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
gboolean on_tree_item_folder_button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data);

void on_drag_data_received (GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *selection_data, guint info, guint time);

void on_tree_item_expand 	(GtkTreeItem* tree_item, gpointer user_data);
void on_tree_item_collapse 	(GtkTreeItem* tree_item, gpointer user_data);

void on_tree_item_deselect 	(GtkTreeItem* item, gpointer user_data);
void on_tree_item_select 	(GtkTreeItem* item, gpointer user_data);

