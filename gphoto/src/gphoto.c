#include "gphoto.h"
#include "main.h"
#include <stdio.h>

extern GtkWidget *status_bar;
extern GtkWidget *progress;

void update_status(char *newStatus) {

        /*
                displays whatever is in string "newStatus" in the
                status bar at the bottom of the main window
        */
    
        if (command_line_mode) {
	    fprintf(stderr,"%s\n",newStatus);
	    return;
        }

        gtk_label_set(GTK_LABEL(status_bar), newStatus);
        while (gtk_events_pending())
	    gtk_main_iteration();
}

void update_progress(float percentage) {

        /*
                sets the progress bar to percentage% at the bottom
                of the main window
        */

	if (command_line_mode) {
	    fprintf(stderr, "#");
	    fflush(stderr);
	    return;
	}

	if (percentage >= 0)
	        gtk_progress_bar_update(GTK_PROGRESS_BAR(progress), percentage);
        while (gtk_events_pending())
                gtk_main_iteration();
}
