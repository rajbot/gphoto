#include "main.h"
#include "gphoto.h"
#include "gallery.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>  


/* HTML Gallery Engine for generating a web gallery of your images
   from within gPhoto. Can be used with any camera library, or even
   with "browse directory" to create your html gallery...

	Gallery tags:
		GALLERY_NAME
		GALLERY_INDEX

		DATE

		THUMBNAIL
		THUMBNAIL_FILENAME
		THUMBNAIL_NUMBER

		PICTURE
		PICTURE_FILENAME
		PICTURE_NUMBER

		PICTURE_NEXT
		PICTURE_PREVIOUS
*/

#include <sys/dir.h>

GtkWidget *filesel;

/* initialize some tags */
char gallery_name[256];
char gallery_index[128];
char date[32];
char thumbnail[256];
char thumbnail_filename[128];
char thumbnail_number[5];
char picture[256];
char picture_filename[128];
char picture_number[5];
char picture_next[128];
char picture_previous[128];

void gallery_change_dir(GtkWidget *widget, GtkWidget *label) {

        filesel = gtk_file_selection_new("Select an Output Directory...");

        gtk_widget_hide(GTK_FILE_SELECTION(filesel)->selection_entry);
        gtk_widget_hide(GTK_FILE_SELECTION(filesel)->selection_text);
        gtk_widget_hide(GTK_FILE_SELECTION(filesel)->file_list);
	gtk_file_selection_set_filename(GTK_FILE_SELECTION(filesel),
		gtk_object_get_data(GTK_OBJECT(label), "dir"));

        if (wait_for_hide(filesel,
           GTK_FILE_SELECTION(filesel)->ok_button,
           GTK_FILE_SELECTION(filesel)->cancel_button) == 0) {
		return;
	}

	gtk_label_set(GTK_LABEL(label),
		gtk_file_selection_get_filename(GTK_FILE_SELECTION(filesel)));
	gtk_object_set_data(GTK_OBJECT(label), "dir",
		gtk_file_selection_get_filename(GTK_FILE_SELECTION(filesel)));
}

void gallery_parse_tags(char *dest, char *source) {

	char fname[1024];
	sprintf(fname,
"sed 's/#GALLERY_NAME#/%s/; \
s/#GALLERY_INDEX#/%s/; \
s/#DATE#/%s/; \
s/#THUMBNAIL#/%s/; \
s/#THUMBNAIL_FILENAME#/%s/; \
s/#THUMBNAIL_NUMBER#/%s/; \
s/#PICTURE#/%s/; \
s/#PICTURE_FILENAME#/%s/; \
s/#PICTURE_NUMBER#/%s/; \
s/#PICTURE_NEXT#/%s/; \
s/#PICTURE_PREVIOUS#/%s/;' %s >> %s",
gallery_name, gallery_index, date, thumbnail, thumbnail_filename,
thumbnail_number, picture, picture_filename, picture_number,
picture_next, picture_previous, source, dest);

/*	printf("%s", fname);*/
	system(fname);
}

void gallery_main() {

	DIR *dir, *testdir;
	struct dirent *file;
	time_t loctime;

	int i=0, j=0, num_selected = 0;
	char outputdir[1024], theme[32];
	char filename[1024], filename2[1024], cp[1024];
	GList *dlist;

	GtkWidget *dialog, *list, *list_item, *scrwin, *obutton, *cbutton;
	GtkWidget *galentry;
	GtkWidget *label, *dirlabel, *dirbutton, *hbox, *hseparator;
	GdkImlibImage *imlibimage;

	struct ImageInfo *node = &Thumbnails;
	struct ImageInfo *node_image;

	if (Thumbnails.next == NULL) {
		error_dialog("Please retrieve the index first,
and select the images to include\nin the gallery by clicking on them.
Then, re-run the HTML Gallery.");
		return;
	}

	/* make sure there's something selected */
	i = 0;
	while (node->next != NULL) {
		node = node->next;
		if (GTK_TOGGLE_BUTTON(node->button)->active)
			num_selected++;
	}
	if (num_selected == 0) {
		error_dialog(
			"You need to select images to add\nto the gallery.");
		return;
	}

	dialog = gtk_dialog_new();
	gtk_widget_set_usize(dialog, 400, 300);
	gtk_container_border_width(GTK_CONTAINER(dialog), 5);

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_widget_show(hbox);
	gtk_widget_set_usize(hbox, 390, 25);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox,
		FALSE, FALSE, 0);

	label = gtk_label_new("Gallery Name:");
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

	galentry = gtk_entry_new();
	gtk_widget_show(galentry);
	gtk_box_pack_start_defaults(GTK_BOX(hbox), galentry);

	hseparator = gtk_hseparator_new();
	gtk_widget_show(hseparator);
	gtk_widget_set_usize(hseparator, 390, 5);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hseparator,
		FALSE, FALSE, 0);

	label = gtk_label_new("Theme:");
	gtk_widget_show(label);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
	gtk_widget_set_usize(label, 390, 25);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label,
		FALSE, FALSE, 0);

	list = gtk_list_new();
        gtk_widget_show(list);
        gtk_list_set_selection_mode (GTK_LIST(list),GTK_SELECTION_SINGLE);

	dir = opendir("/usr/share/gphoto/gallery");
	if (dir == NULL) {
		error_dialog(
"HTML gallery themes do not exist at
/usr/share/gphoto/gallery
Please install move gallery themes there.");
		return;
	}

	file = readdir(dir);
	while (file != NULL) {
		sprintf(filename, "/usr/share/gphoto/gallery/%s",
			file->d_name);
		testdir = opendir(filename);
                if ((strcmp(file->d_name, ".") != 0) &&
                    (strcmp(file->d_name, "..") != 0) &&
		    (testdir != NULL)) {
			closedir(testdir);
	                list_item = gtk_list_item_new_with_label(file->d_name);
	                gtk_widget_show(list_item);
        	        gtk_container_add(GTK_CONTAINER(list), list_item);
                	gtk_object_set_data(GTK_OBJECT(list_item),"theme",
                                        file->d_name);
		}
		file = readdir(dir);
	}

        scrwin = gtk_scrolled_window_new(NULL, NULL);
        gtk_widget_show(scrwin);
#ifndef GTK_HAVE_FEATURES_1_1_4
        gtk_container_add(GTK_CONTAINER(scrwin), list);
#else 
        gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW(scrwin), list);
#endif
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(index_window),
                                        GTK_POLICY_AUTOMATIC,
                                        GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		scrwin);

	hseparator = gtk_hseparator_new();
	gtk_widget_show(hseparator);
	gtk_widget_set_usize(hseparator, 390, 5);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hseparator,
		FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_widget_show(hbox);
	gtk_widget_set_usize(hbox, 390, 25);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox,
		FALSE, FALSE, 0);	

	label = gtk_label_new("Output Directory:");
	gtk_widget_show(label);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

	dirlabel = gtk_label_new("./");
	gtk_widget_show(dirlabel);
	gtk_label_set_justify(GTK_LABEL(dirlabel), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start_defaults(GTK_BOX(hbox), dirlabel);
	gtk_object_set_data(GTK_OBJECT(dirlabel), "dir", "./");

	dirbutton = gtk_button_new_with_label("Change...");
	gtk_widget_show(dirbutton);
	gtk_signal_connect_object(GTK_OBJECT(dirbutton), "clicked",
		GTK_SIGNAL_FUNC(gallery_change_dir),GTK_OBJECT(dirlabel));
	gtk_box_pack_end(GTK_BOX(hbox), dirbutton, FALSE, FALSE, 0);

	obutton = gtk_button_new_with_label("Create");
	gtk_widget_show(obutton);
        GTK_WIDGET_SET_FLAGS (obutton, GTK_CAN_DEFAULT);
	gtk_box_pack_start_defaults(
		GTK_BOX(GTK_DIALOG(dialog)->action_area), obutton);

	cbutton = gtk_button_new_with_label("Cancel");
	gtk_widget_show(cbutton);
	gtk_box_pack_start_defaults(
		GTK_BOX(GTK_DIALOG(dialog)->action_area), cbutton);
	gtk_widget_show(dialog);
	gtk_widget_grab_default(obutton);

        if (wait_for_hide(dialog, obutton, cbutton) == 0)
                return;

	dlist = GTK_LIST(list)->selection;
	if (!dlist) {
		/* Whoops, didn't select a theme... */
		error_dialog("You must select a theme!");
		return;
	}

	strcpy(cp, "");
	sprintf(theme, "%s",(char*)gtk_object_get_data(GTK_OBJECT(dlist->data),
		"theme"));
	sprintf(gallery_name,"%s",
		gtk_entry_get_text(GTK_ENTRY(galentry)));
	strcpy(gallery_index, "index.html");
	loctime = time(&loctime);
        sprintf(date, "%s", ctime(&loctime));
	date[24] = '\0';
 	sprintf(outputdir,"%s",
		(char*)gtk_object_get_data(GTK_OBJECT(dirlabel),"dir"));

	sprintf(filename, "/usr/share/gphoto/gallery/%s",
		theme);
	dir = opendir(filename);
	file = readdir(dir);
	while (file != NULL) {
		if ((strcmp(file->d_name, "index_top.html") != 0 ) &&
		    (strcmp(file->d_name, "index_bottom.html") != 0 ) &&
		    (strcmp(file->d_name, "picture.html") != 0 ) &&
		    (strcmp(file->d_name, "thumbnail.html") != 0 ) &&
		    (strcmp(file->d_name, ".") != 0 ) &&
		    (strcmp(file->d_name, "..") != 0 )) {
			sprintf(cp, "cp %s/%s %s", filename, file->d_name,
						   outputdir);
			system(cp);
		}
		file = readdir(dir);
	}	
	closedir(dir);
	sprintf(filename, "%sindex.html", outputdir);	
	sprintf(cp, "echo \" \" > %s", filename);
	system(cp);
	sprintf(filename2,
		"/usr/share/gphoto/gallery/%s/index_top.html",
		theme);	
	gallery_parse_tags(filename, filename2);
	sprintf(cp, "echo \"<table border=1><tr>\" >> %s", filename);
	system(cp);
	node = &Thumbnails;
	while (node->next != NULL) {
		int loaded;	/* flag, set if image had been downloaded */
		int picnum;	/* number of the downloaded image */
		node = node->next;
		sprintf(filename, "%sindex.html", outputdir);	
		if (GTK_TOGGLE_BUTTON(node->button)->active) {
			if ((i%5==0) && (i!=0)) {
				sprintf(cp, "echo \"</tr><tr>\" >> %s",
					filename);
				system(cp);
			}
			sprintf(thumbnail, 
	"<a href=\"picture-%03i.html\"><img src=\"thumbnail-%03i.jpg\"><\\/a>",
				i+1, i+1);
			sprintf(thumbnail_filename, "thumbnail-%03i.jpg",
				i+1);
			sprintf(thumbnail_number, "%03i", i+1);
			sprintf(picture, "<img src=\"picture-%03i.jpg\">",
				i+1);
			sprintf(picture_filename, "picture-%03i.jpg",
				i+1);
			sprintf(picture_number, "%03i", i+1);
			if (i+1 == num_selected)
				strcpy(picture_next, gallery_index);
			   else	
				sprintf(picture_next, "picture-%03i.html",
					 i+2);
			if (i == 0)
				strcpy(picture_previous, gallery_index);
			   else
				sprintf(picture_previous, "picture-%03i.html",
					i);
			sprintf(cp, "echo \"<td>\" >> %s", filename);
			system(cp);
			
			sprintf(filename2,
				"/usr/share/gphoto/gallery/%s/thumbnail.html",
				theme);
			gallery_parse_tags(filename, filename2);
			sprintf(cp, "echo \"</td>\" >> %s", filename);
			system(cp);
			sprintf(filename2, "%s%s", outputdir,
				thumbnail_filename);
			sprintf(cp, "Getting Image #%i...", j+1);
			update_status(cp);
			/* see if image already downloaded */
			for(node_image = &Images, loaded = 0;
			  node_image->next != NULL;) {
			    node_image = node_image->next;
			    sscanf(node_image->info,"%i",&picnum);
			    if(picnum == j + 1) {
			    	loaded = 1;
				imlibimage = node_image->imlibimage;
			    	break;
			    }
			}
			if(loaded == 0) {
                            appendpic(j + 1, TRUE, NULL);
			    /* now find the just loaded image.
			     * this seems dumb, appendpic should just
			     * return the image node. cliff */
			    for(node_image = &Images;
			      node_image->next !=NULL;) {
				node_image = node_image->next;
				sscanf(node_image->info,"%i",&picnum);
				if(picnum == j + 1) {
				    imlibimage = node_image->imlibimage;
				    break;
				}
			    }
			}
			gtk_toggle_button_set_state(
                          GTK_TOGGLE_BUTTON(node->button), FALSE);
			gdk_imlib_save_image(node->imlibimage, filename2,
				NULL);
			sprintf(filename2, "%s%s", outputdir,
				picture_filename);
			update_status(filename2);
			gdk_imlib_save_image(imlibimage, filename2, NULL);
			gdk_imlib_kill_image(imlibimage);
			sprintf(filename, "%spicture-%03i.html",
				outputdir, i+1);
			sprintf(cp, "echo \" \" > %s", filename);
			system(cp);
	
			sprintf(filename2,
				"/usr/share/gphoto/gallery/%s/picture.html",
				theme);
			gallery_parse_tags(filename, filename2);
			i++;
		}
		j++;
	}
	sprintf(filename, "%sindex.html", outputdir);	
	sprintf(cp, "echo \"</tr></table>\" >> %s", filename);
	system(cp);
	sprintf(filename2,
		"/usr/share/gphoto/gallery/%s/index_bottom.html",
		theme);			
	gallery_parse_tags(filename, filename2);
	gtk_widget_destroy(filesel);
	gtk_widget_destroy(dialog);
	update_status("Done saving Gallery.");
}
