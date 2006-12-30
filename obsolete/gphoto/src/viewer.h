#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gdk_imlib.h>

void view_image_window(gchar *file);
GtkWidget *view_image_fullscreen(gchar *file, GtkWidget *win);
void view_images(GList *list);

void change_image(GtkWidget *win, int change);
