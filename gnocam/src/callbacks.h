
gboolean on_tree_cameras_button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data);
void on_drag_data_received (GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *selection_data, guint info, guint time);

void on_tree_item_expand (GtkTreeItem* tree_item, gpointer user_data);
void on_tree_item_collapse (GtkTreeItem* tree_item, gpointer user_data);
