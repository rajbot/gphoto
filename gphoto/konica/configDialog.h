GtkWidget *qm100_getWidget(GtkWidget *widget, gchar *widget_name);
void       qm100_setNotebookTab(GtkWidget *notebook, gint page_num, GtkWidget *widget);
void       qm100_addPixmapDirectory(gchar *directory);
GtkWidget *qm100_createPixmap(GtkWidget *widget, gchar *filename);
GtkWidget *qm100_createConfigDlg (void);

void on_okBtn_clicked(GtkButton *button, gpointer user_data);
void setSize(GtkButton *button, gpointer user_data);
void on_spd_pressed(GtkButton *button, gpointer user_data);
void cancel_btn_clicked(GtkButton *button, gpointer user_data);


