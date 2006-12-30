#include "viewer.h"
#include <stdio.h>

/* closes window w and destroys any static reference.
 * i.e. the same window used in fullscreen mode */
void close_window(GtkWidget *w, gpointer *data)
{
	/* get the image if this is viewed in a toplevel window */
	GdkImlibImage *im = NULL;
	im = (GdkImlibImage *) gtk_object_get_data(GTK_OBJECT(w), "image");
	if (im)
    	gdk_imlib_destroy_image(im);

	gtk_widget_destroy(w);
	if (data)
		*data = NULL;
}

/* handles mouse click events for fullscreen and windowed images.*/
void click_handler(GtkWidget *w, GdkEventButton *event, gpointer *data)
{
	switch (event->button)
	{
		case 2:
			close_window(w, data);
			break;
		case 1:
			change_image(w, 1);
			break;
		case 3:
			change_image(w, -1);
			break;
	}
}

/* handles key press events for fullscreen and windowed images.*/
int key_handler(GtkWidget *w, GdkEventKey *event, gpointer *data)
{
	switch (event->keyval)
	{
		case GDK_q:
		case GDK_Q:
		case GDK_Escape:
			close_window(w, data);
			return TRUE;
		case GDK_n:
		case GDK_N:
		case GDK_space:
			change_image(w, 1);
			return TRUE;
		case GDK_p:
		case GDK_P:
			change_image(w, -1);
			return TRUE;
		default:
			break;
	}
	return FALSE;
}

void image_resize_cb(GtkWidget *widget, GdkEventConfigure *event)
{
	gint w, h;
	static gint pw = -1, ph = -1;
	GtkWidget *a;
	GdkImlibImage *im;
	GdkPixmap *pmap;

	w = event->width;
	h = event->height;

	if ((pw == w) && (ph == h))
	  return;

	pw = w;
	ph = h;

	im = (GdkImlibImage*) gtk_object_get_data(GTK_OBJECT(widget), "image");
	a = (GtkWidget*) gtk_object_get_data(GTK_OBJECT(widget), "view");
	if (!a || !im)
		return;

	gdk_imlib_render(im, w, h);
	pmap = gdk_imlib_move_image(im);
	gdk_window_set_back_pixmap(a->window, pmap, FALSE);
	gdk_window_clear(a->window);
	gdk_imlib_free_pixmap(pmap);
}


/* moves to the next or previous image in the list,
 * which is data attached to win */
void change_image(GtkWidget *win, int change)
{
	GList *list, *new;
	static GList *iterator = NULL;

	list = (GList*) gtk_object_get_user_data(GTK_OBJECT(win));

	if (list)
	{
		if (!iterator)
				iterator = list;

		if (change > 0)
				new = iterator->next;
		else
				new = iterator->prev;

		if (new)
		{
			iterator = new;
			view_image_fullscreen((gchar*)iterator->data, win);
		}
	}
}

/* list contains a list of files to view */
void view_images(GList *list)
{
	GtkWidget *win = NULL;

	if (list)
	{
		win = view_image_fullscreen((gchar *)list->data, win);
		if (win)
			gtk_object_set_user_data(GTK_OBJECT(win), (gpointer) list);
	}
}

/* views a single file fullscreen. if win is NULL, a new window
 * will be created. Otherwise, the image will be displayed in the
 * existing window win */
GtkWidget *view_image_fullscreen(gchar *file, GtkWidget *win)
{
	GdkImlibImage *im;
	static GtkWidget *draw_area = NULL;
	GdkPixmap *pmap;
	int width, height, screen_x, screen_y, win_x, win_y;

	im = gdk_imlib_load_image(file);

	if (!im)
		return NULL;

	width = im->rgb_width;
	height = im->rgb_height;
	screen_x=gdk_screen_width();
	screen_y=gdk_screen_height();
	win_x=(screen_x - width) / 2;
	win_y=(screen_y - height) / 2;

	gdk_imlib_render(im, width, height);
	pmap = gdk_imlib_move_image(im);

	if (!win)
	{
		draw_area = gtk_drawing_area_new();
		gtk_drawing_area_size (GTK_DRAWING_AREA (draw_area), screen_x, screen_y);
		gtk_widget_show(draw_area);

		win = gtk_window_new(GTK_WINDOW_POPUP);
		gtk_window_set_modal(GTK_WINDOW(win), TRUE);

		gtk_signal_connect( GTK_OBJECT (win), "delete_event",
					GTK_SIGNAL_FUNC (close_window), &win);
		gtk_widget_set_events(win, GDK_BUTTON_PRESS_MASK);
		gtk_signal_connect( GTK_OBJECT (win), "key_press_event",
					GTK_SIGNAL_FUNC (key_handler), &win);
		gtk_signal_connect(GTK_OBJECT(win), "button_press_event",
					GTK_SIGNAL_FUNC (click_handler), &win);
		gtk_container_set_border_width( GTK_CONTAINER (win), 0 );
	
		gtk_widget_set_usize(win, screen_x, screen_y);
		gtk_widget_show( win );
		gtk_container_add(GTK_CONTAINER(win), draw_area);
	}

	while (gtk_events_pending())
		gtk_main_iteration();

	gdk_window_set_background(draw_area->window, &draw_area->style->black);
	gdk_window_clear(draw_area->window);
	gdk_draw_pixmap(draw_area->window, draw_area->style->fg_gc[draw_area->state], pmap, 0, 0, win_x, win_y, width, height);
	gdk_flush();
	
	gdk_imlib_free_pixmap(pmap);
	gdk_imlib_destroy_image(im);

	return win;
}

/* views a file and creates a new window */
void view_image_window (gchar *file)
{
	GdkImlibImage *im;
	GtkWidget *win, *draw_area, *sc_win;
	GdkPixmap *pmap;
	int width, height;

	im = gdk_imlib_load_image(file);

	if (!im)
		return;

	width = im->rgb_width;
	height = im->rgb_height;

	gdk_imlib_render(im, width, height);
	pmap = gdk_imlib_move_image(im);

	draw_area = gtk_drawing_area_new();
	gtk_widget_show(draw_area);

	win = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_signal_connect( GTK_OBJECT (win), "delete_event",
                    GTK_SIGNAL_FUNC (close_window), NULL );
	gtk_widget_set_events(win, GDK_BUTTON_PRESS_MASK);
	gtk_signal_connect( GTK_OBJECT (win), "key_press_event",
                    GTK_SIGNAL_FUNC (key_handler), NULL);
	gtk_signal_connect(GTK_OBJECT(win), "button_press_event",
					GTK_SIGNAL_FUNC (click_handler), NULL);
	gtk_signal_connect(GTK_OBJECT(win), "configure_event",
					GTK_SIGNAL_FUNC (image_resize_cb), NULL);

	gtk_object_set_data(GTK_OBJECT(win), "image", (gpointer)im);
	gtk_object_set_data(GTK_OBJECT(win), "view", (gpointer)draw_area);

	gtk_container_set_border_width( GTK_CONTAINER (win), 0 );
	
	gtk_window_set_default_size(GTK_WINDOW(win), width, height);
	gtk_window_set_title(GTK_WINDOW(win), file);

	gtk_widget_show( win );
	gtk_container_add(GTK_CONTAINER(win), draw_area);

	while (gtk_events_pending())
		gtk_main_iteration();

	gdk_window_set_back_pixmap(draw_area->window, pmap, FALSE);
	gdk_window_clear(draw_area->window);
	
	gdk_imlib_free_pixmap(pmap);
}
