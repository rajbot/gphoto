#include "gphoto.h"
#include "main.h"
#include <stdio.h>

extern GtkWidget *status_bar;
extern GtkWidget *progress;

void update_status(char *newStatus)
{
	/*
	 * displays whatever is in string "newStatus" in the
	 * status bar at the bottom of the main window
         */
	if (command_line_mode) {
		fprintf(stdout,"%s\n",newStatus);
		return;
	}

	gtk_label_set(GTK_LABEL(status_bar), newStatus);
	while (gtk_events_pending())
		gtk_main_iteration();
}

void update_progress(int percentage)
{
	/*
	 * sets the progress bar to percentage% at the bottom
	 *of the main window
	 */
	float p;

	if (command_line_mode) {
		fprintf(stdout, "#");
		fflush(stdout);
		return;
	}

	p = (float)percentage/100.0;
	if ((p >= 0.0) && (p <= 1.0))
		gtk_progress_bar_update(GTK_PROGRESS_BAR(progress), p);

	while (gtk_events_pending())
		gtk_main_iteration();
}
