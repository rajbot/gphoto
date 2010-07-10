/* gtkam-save.c
 *
 * Copyright 2001,2002      Lutz Mueller <lutz@users.sf.net>
 * Copyright 2003,2004,2005 Andrew Burton <adburton@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "gtkam-save.h"
#include "i18n.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkframe.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtktooltips.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtklabel.h>

#include <gphoto2/gphoto2-setting.h>

#include "util.h"

#include "gtkam-cancel.h"
#include "gtkam-close.h"
#include "gtkam-error.h"
#include "gtkam-status.h"

typedef struct _GtkamSaveData GtkamSaveData;
struct _GtkamSaveData {
	GtkamCamera *camera;
	gchar *folder;
	gchar *name;
};


struct _GtkamSavePrivate
{
	GSList *data;
	GString *filelist;

	GtkToggleButton *toggle_preview, *toggle_normal, *toggle_raw,
			*toggle_audio, *toggle_exif;
	GtkToggleButton *toggle_filename_camera;
	GtkEntry *program;

	GtkWidget *spin_entry, *prefix_entry, *hbox_prefix;

	gboolean quiet, err_shown;

	GtkWidget *status;
	GtkWindow *main_window;
};

#define PARENT_TYPE GTK_TYPE_FILE_CHOOSER_DIALOG
static GtkFileChooserDialog *parent_class;

static void
gtkam_save_destroy (GtkObject *object)
{
	GtkamSave *save = GTKAM_SAVE (object);
	gint i;
	GtkamSaveData *data;

	if (save->priv->data) {
		for (i = g_slist_length (save->priv->data) - 1; i >= 0; i--) {
			data = g_slist_nth_data (save->priv->data, i);
			g_object_unref (G_OBJECT (data->camera));
			g_free (data->folder);
			g_free (data->name);
			g_free (data);
		}
		g_slist_free (save->priv->data);
		save->priv->data = NULL;
	}

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gtkam_save_finalize (GObject *object)
{
	GtkamSave *save = GTKAM_SAVE (object);

	g_free (save->priv);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gtkam_save_class_init (gpointer g_class, gpointer class_data)
{
	GtkObjectClass *object_class;
	GObjectClass *gobject_class;

	object_class = GTK_OBJECT_CLASS (g_class);
	object_class->destroy  = gtkam_save_destroy;

	gobject_class = G_OBJECT_CLASS (g_class);
	gobject_class->finalize = gtkam_save_finalize;

	parent_class = g_type_class_peek_parent (g_class);
}

static void
gtkam_save_init (GTypeInstance *instance, gpointer g_class)
{
	GtkamSave *save = GTKAM_SAVE (instance);

	save->priv = g_new0 (GtkamSavePrivate, 1);
}

GType
gtkam_save_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo ti;

		memset (&ti, 0, sizeof (GTypeInfo));
		ti.class_size     = sizeof (GtkamSaveClass);
		ti.class_init     = gtkam_save_class_init;
		ti.instance_size  = sizeof (GtkamSave);
		ti.instance_init  = gtkam_save_init;

		type = g_type_register_static (PARENT_TYPE, "GtkamSave",
				&ti, 0);
	}

	return (type);
}

static void
on_cancel_clicked (GtkButton *button, GtkamSave *save)
{
	gtk_widget_destroy (GTK_WIDGET (save));
}

static int
default_numbering_start(const GtkamSave *save)
{
	GDir *dir;
	const gchar *file;
	gchar *tail,*prefix;
	int max = 1;
	int plen, current;
	gchar *filename ;
	
	filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (save));
	if (filename == NULL) return 1;
	if (!(dir = g_dir_open (filename , 0, NULL))) return 1;
	prefix = g_strdup (gtk_entry_get_text
					(GTK_ENTRY (save->priv->prefix_entry)));
	plen = strlen (prefix);
	while ((file = g_dir_read_name (dir))) {
		/* compare prefix */
		if (strncmp (prefix, file, plen)) continue;
		/* check number */
		current = strtol (file+plen, &tail, 10);
		if (tail - (file + plen) != 4) continue;
		/* FIXME: check for regular file here */
		if (max <= current) max = current + 1;
	}
	
	g_free (prefix);
	g_dir_close (dir);
	return max;
}

static void
on_filename_camera_toggled (GtkToggleButton *toggle, GtkamSave *save)
{
	/*
	 * Give the users the possibility to specify prefix and numbering.
	 * No direct entry of filename is allowed.
	 */
	gtk_widget_show (save->priv->hbox_prefix);
	gtk_widget_set_sensitive (GTK_WIDGET (save->priv->prefix_entry),
			!toggle->active);
	gtk_widget_set_sensitive (GTK_WIDGET (save->priv->spin_entry),
			!toggle->active);
	if (!toggle->active) {
		/* Calculate default start number */
		gtk_spin_button_set_value (
			GTK_SPIN_BUTTON (save->priv->spin_entry),
			default_numbering_start (save));
	}
}

static gchar *
create_full_filename (const gchar *filename, CameraFileType type)
{
	gchar *full_filename;

	switch (type) {
	case GP_FILE_TYPE_PREVIEW:
		full_filename = g_strdup_printf ("thumb_%s", filename);
		break;
	case GP_FILE_TYPE_RAW:
		full_filename = g_strdup_printf ("raw_%s", filename);
		break;
	case GP_FILE_TYPE_AUDIO:
		full_filename = g_strdup_printf ("audio_%s", filename);
		break;
	case GP_FILE_TYPE_EXIF:
		full_filename = g_strdup_printf ("exif_%s", filename);
		break;
	default:
		full_filename = g_strdup (filename);
		break;
	}

	return (full_filename);
}

static gchar *
concat_dir_and_file (const gchar *dirname, const gchar *filename)
{
	gchar *full_path;

	if (!strcmp (dirname, "/"))
		full_path = g_strdup_printf ("/%s", filename);
	else if (dirname[strlen (dirname) - 1] == '/')
		full_path = g_strdup_printf ("%s%s", dirname, filename);
	else
		full_path = g_strdup_printf ("%s/%s", dirname, filename);
	return (full_path);
}

static int
save_file (GtkamSave *save, const char *filename, CameraFile *file, CameraFileType type, guint n)
{
	gchar *full_path, *full_filename, *dirname, *msg, *number_filename;
	const char *mime_type;
	const gchar *fsel_filename, *fsel_path, *prefix, *suffix;
	GtkWidget *dialog;
	int result;

	gp_file_get_mime_type (file, &mime_type);

	fsel_path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER_DIALOG (save));

	if ((save->priv->toggle_filename_camera->active) ||
		(g_slist_length (save->priv->data) == 1)) {

		/* Use filename provided by the CameraFile */
		full_filename = create_full_filename (filename, type);

		if (g_slist_length (save->priv->data) == 1)
			full_path = g_strdup(fsel_path);
		else
			full_path = concat_dir_and_file (fsel_path, full_filename);

		g_free (full_filename);

	} else {
		/* Use filename in prefix */
		prefix = g_locale_from_utf8 (gtk_entry_get_text
					(GTK_ENTRY (save->priv->prefix_entry)), -1,
					NULL, NULL, NULL);

		suffix = strrchr (mime_type, '/');
		suffix++;
		number_filename = g_strdup_printf ("%s%04i.%s", prefix,
								n, suffix);
		full_filename = create_full_filename (number_filename,
								type);
		g_free (number_filename);
		
		full_path = concat_dir_and_file (fsel_path,
							full_filename);
		g_free (full_filename);
	}

	/* FIXME Check which is user, and prompt the user */
	if (!save->priv->quiet && file_exists (full_path)) {
		msg = g_strdup_printf (_("The file '%s' already exists."),
				       full_path);
		if (!save->priv->err_shown) {
			dialog = gtkam_error_new (GP_ERROR_FILE_EXISTS, NULL, GTK_WIDGET (save), msg);
			gtk_window_set_transient_for (GTK_WINDOW (dialog),
						      save->priv->main_window);
			gtk_widget_show (dialog);
			g_free (msg);
			g_free (full_path);
			save->priv->err_shown = TRUE;
		}
		return -1;
	}

	/* FIXME Check for sufficient disk space for this file, or
	   calculate total disk space required for all files before
	   save process starts */

	result = gp_file_save (file, full_path);
	if (result < 0) {
		if (!save->priv->err_shown) {
			dialog = gtkam_error_new (result, NULL, GTK_WIDGET (save),
					_("Could not save file to '%s'."), full_path);
			gtk_widget_show (dialog);
			save->priv->err_shown = TRUE;
		}
	} else {
		/* Add the file to the list of files to be opened with
		   the specified external program */
		if (!save->priv->filelist)
				save->priv->filelist = g_string_new ("");
		g_string_append_printf (save->priv->filelist, " %s", full_path);
	}

	g_free (full_path);
	
	return result;
}

static int
get_file (GtkamSave *save, GtkamCamera *camera,
	  const gchar *folder, const gchar *name, CameraFileType type, guint n,
	  GtkamContext *context)
{
	int result;
	GtkWidget *dialog;
	CameraFile *file;

	gp_file_new (&file);
	result = gp_camera_file_get (camera->camera, folder, name, type, file,
				     context->context);
	if (camera->multi)
		gp_camera_exit (camera->camera, NULL);
	switch (result) {
	case GP_OK:
		result = save_file (save, name, file, type, n);
		break;
	case GP_ERROR_CANCEL:
		break;
	default:
		if (!save->priv->err_shown) {
			dialog = gtkam_error_new (result, context,
				GTK_WIDGET (save), _("Could not get '%s' "
				"from folder '%s'."),
				name, folder);
			gtk_widget_show (dialog);
			save->priv->err_shown = TRUE;
		}
	}
	gp_file_unref (file);

	return result;
}

static void
store_save_settings(GtkamSave *save)
{
	gchar *savedir,*t;
	gchar buf[5];

	/* Directory */
	savedir = g_strdup (gtk_file_chooser_get_filename (
			      GTK_FILE_CHOOSER_DIALOG (save)));
	if (savedir) {
		if (strlen (savedir) > 255)
			savedir[255] = '\0';

		if (!g_file_test (savedir, G_FILE_TEST_IS_DIR)) {
			t = g_path_get_dirname (savedir);
			g_free (savedir);
			savedir = t;
		}
		gp_setting_set ("gtkam", "save-dir", savedir);
		g_free (savedir);
	}
	
	/* toggle buttons */
#define STORE_TOGGLE(NAME) \
	snprintf(buf,sizeof(buf), "%d", gtk_toggle_button_get_active(	\
		save->priv->toggle_ ## NAME)); 				\
	gp_setting_set("gtkam","save-" #NAME,buf)
	
	STORE_TOGGLE(preview);
	STORE_TOGGLE(normal);
	STORE_TOGGLE(raw);
	STORE_TOGGLE(audio);
	STORE_TOGGLE(exif);
	STORE_TOGGLE(filename_camera);

	/* prefix entry */
	if (g_slist_length (save->priv->data) > 1) {
		/* only when saving several files */
		gp_setting_set("gtkam", "save-prefix", (gchar *)
			gtk_entry_get_text (GTK_ENTRY (save->priv->prefix_entry)));
	}
}

static void
load_save_settings(GtkamSave *save)
{
	gchar buf[1024];
	gchar *dir;

	/* directory */
	if (gp_setting_get("gtkam","save-dir",buf) == GP_OK) {
		dir = g_strdup (buf);
		strcat (buf, G_DIR_SEPARATOR_S);
		gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (save), buf);
	} else {
		dir = g_strdup (gtk_file_chooser_get_filename (
			GTK_FILE_CHOOSER (save)));
	}
	/* prefix entry */
	gp_setting_get ("gtkam", "save-prefix", buf);
	gtk_entry_set_text (GTK_ENTRY (save->priv->prefix_entry), buf);

	/* toggle buttons */
#define LOAD_TOGGLE(NAME) \
	gp_setting_get("gtkam","save-" #NAME,buf); 			\
	gtk_toggle_button_set_active(save->priv->toggle_ ## NAME, atoi(buf))

	LOAD_TOGGLE(preview);
	LOAD_TOGGLE(normal);
	LOAD_TOGGLE(raw);
	LOAD_TOGGLE(audio);
	LOAD_TOGGLE(exif);
	LOAD_TOGGLE(filename_camera);
	g_free(dir);
}

static int
count_items (GtkamSave *save)
{
	int items_to_save = 0;
	
	if (save->priv->toggle_normal &&
		    save->priv->toggle_normal->active)
		items_to_save++;

	if (save->priv->toggle_preview &&
	    save->priv->toggle_preview->active)
		items_to_save++;

	if (save->priv->toggle_raw &&
	    save->priv->toggle_raw->active)
		items_to_save++;

	if (save->priv->toggle_audio &&
	    save->priv->toggle_audio->active)
		items_to_save++;

	if (save->priv->toggle_exif &&
	    save->priv->toggle_exif->active)
		items_to_save++;

	return items_to_save;
}

static void
on_ok_clicked (GtkButton *button, GtkamSave *save)
{
	guint i, count, j = 1;
	int result = -1;
	GtkWidget *s, *dialog;
	unsigned int id = 0;
	GtkamSaveData *data;
	gchar *progname, *command;
	GError *error = NULL;

	if (count_items (save) == 0) {
		if (!save->priv->err_shown) {
			
			dialog = gtkam_error_new (result, NULL,
					GTK_WIDGET (save), _("There is nothing to be saved."));
			gtk_widget_show (dialog);
		}

		return;
	}
			
	store_save_settings(save);
	gtk_widget_hide (GTK_WIDGET (save));

	count = g_slist_length (save->priv->data);
	if (count == 1)
		s = gtkam_cancel_new (_("Downloading file"));
	else
		s = gtkam_cancel_new (_("Downloading %i files"), count);
	gtk_window_set_transient_for (GTK_WINDOW (s), save->priv->main_window);
	gtk_widget_show (s);

	if (count > 1)
		id = gp_context_progress_start (
			GTKAM_CANCEL (s)->context->context, count,
			_("Downloading %i files..."), count);

	if (!save->priv->toggle_filename_camera->active)
		j = gtk_spin_button_get_value_as_int (
				GTK_SPIN_BUTTON (save->priv->spin_entry));
	
	for (i = 0; i < count; i++) {
		data = g_slist_nth_data (save->priv->data, i);

		/* Check for shutdown */
		if (!GTKAM_IS_SAVE (save))
			return;

		if (save->priv->toggle_normal &&
		    save->priv->toggle_normal->active)
			result = get_file (save, data->camera,
				  data->folder, data->name,
				  GP_FILE_TYPE_NORMAL, i + j,
				  GTKAM_CANCEL (s)->context);
		if (save->priv->toggle_preview &&
		    save->priv->toggle_preview->active &&
		    (!result < 0))
			result = get_file (save, data->camera,
				  data->folder, data->name,
				  GP_FILE_TYPE_PREVIEW, i + j,
				  GTKAM_CANCEL (s)->context);
		if (save->priv->toggle_raw &&
		    save->priv->toggle_raw->active &&
		    (!result < 0))
			result = get_file (save, data->camera,
				  data->folder, data->name, GP_FILE_TYPE_RAW,
				  i + j, GTKAM_CANCEL (s)->context);
		if (save->priv->toggle_audio &&
		    save->priv->toggle_audio->active &&
		    (!result < 0))
			result = get_file (save, data->camera,
				  data->folder, data->name, GP_FILE_TYPE_AUDIO,
				  i + j, GTKAM_CANCEL (s)->context);
		if (save->priv->toggle_exif &&
		    save->priv->toggle_exif->active &&
		    (!result < 0))
			result = get_file (save, data->camera,
				  data->folder, data->name, GP_FILE_TYPE_EXIF,
				  i + j, GTKAM_CANCEL (s)->context);

		if (result < 0)
		{
			if (count > 1)
				gp_context_progress_stop (GTKAM_CANCEL (s)->context->context, id);
			if (!save->priv->err_shown) {
				dialog = gtkam_error_new (result, GTKAM_CANCEL (s)->context,
						GTK_WIDGET (save), _("Problem getting '%s' "
						"from folder '%s'."),
						data->name, data->folder);
				gtk_widget_show (dialog);
				save->priv->err_shown = TRUE;
			}
	
			gtk_object_destroy (GTK_OBJECT (s));
			gtk_object_destroy (GTK_OBJECT (save));
			return;
		}
		
		if (count > 1)
			gp_context_progress_update (
				GTKAM_CANCEL (s)->context->context, id, i + 1);
		gp_context_idle (GTKAM_CANCEL (s)->context->context);
		if (gp_context_cancel (GTKAM_CANCEL (s)->context->context) ==
				GP_CONTEXT_FEEDBACK_CANCEL)
			break;
	}
	if (count > 1)
		gp_context_progress_stop (
				GTKAM_CANCEL (s)->context->context, id);
	
	gtk_object_destroy (GTK_OBJECT (s));

	/* If file(s) were saved and a program specified, load the program
   passing the filenames */
	if (result >= 0)
	{
		progname = gtk_entry_get_text (save->priv->program);

		if (progname && progname[0] != '\0') {
			command = g_strdup_printf ("%s%s", progname, save->priv->filelist->str);

			/* FIXME Report any arising errors */
			if (!g_spawn_command_line_async (command, &error)) {
				g_warning ("Error running command\n");
				g_error_free (error);
			}
			
			g_free (command);
			g_string_free (save->priv->filelist, TRUE);
		} 
	}
	
	gtk_object_destroy (GTK_OBJECT (save));
}

GtkWidget *
gtkam_save_new (GtkWindow *main_window)
{
	GtkamSave *save;
	GtkWidget *hbox, *frame, *main_vbox, *check, *label, *entry, *button;
	GtkObject *a;
	GtkTooltips *tooltips;

	save = g_object_new (GTKAM_TYPE_SAVE, NULL);

	button = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
	gtk_widget_show (button);
	g_signal_connect (GTK_OBJECT (button),
			  "clicked", GTK_SIGNAL_FUNC (on_cancel_clicked), save);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (save)->action_area),
			   button);

	button = gtk_button_new_from_stock (GTK_STOCK_OK);
	gtk_widget_show (button);
	g_signal_connect (GTK_OBJECT (button), "clicked",
			    GTK_SIGNAL_FUNC (on_ok_clicked), save);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (save)->action_area),
			   button);
	
	tooltips = gtk_tooltips_new ();

	frame = gtk_frame_new (_("What to save:"));
	gtk_widget_show (frame);
	gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER (save), frame);

	main_vbox = gtk_vbox_new (TRUE, 5);
	gtk_widget_show (main_vbox);
	gtk_container_add (GTK_CONTAINER (frame), main_vbox);

	hbox = gtk_hbox_new (TRUE, 5);
	gtk_widget_show (hbox);
	gtk_container_add (GTK_CONTAINER (main_vbox), hbox);

	check = gtk_check_button_new_with_label (_("Save photos"));
	gtk_widget_show (check);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), TRUE);
	gtk_box_pack_start (GTK_BOX (hbox), check, TRUE, TRUE, 0);
	gtk_tooltips_set_tip (tooltips, check, _("Photos will be saved if "
			      "this is checked"), NULL);
	save->priv->toggle_normal = GTK_TOGGLE_BUTTON (check);

	check = gtk_check_button_new_with_label (_("Save raw data"));
	gtk_widget_show (check);
	gtk_box_pack_start (GTK_BOX (hbox), check, TRUE, TRUE, 0);
	gtk_tooltips_set_tip (tooltips, check, _("Raw data will be "
			      "saved if this is checked"), NULL);
	save->priv->toggle_raw = GTK_TOGGLE_BUTTON (check);

	check = gtk_check_button_new_with_label (_("Save audio data"));
	gtk_widget_show (check);
	gtk_box_pack_start (GTK_BOX (hbox), check, TRUE, TRUE, 0);
	gtk_tooltips_set_tip (tooltips, check, _("Audio data will be "
			      "saved if this is checked"), NULL);
	save->priv->toggle_audio = GTK_TOGGLE_BUTTON (check);

	check = gtk_check_button_new_with_label (_("Save thumbnails"));
	gtk_widget_show (check);
	gtk_box_pack_start (GTK_BOX (hbox), check, TRUE, TRUE, 0);
	gtk_tooltips_set_tip (tooltips, check, _("Thumbnails will be "
			      "saved if this is checked"), NULL);
	save->priv->toggle_preview = GTK_TOGGLE_BUTTON (check);

	check = gtk_check_button_new_with_label (_("Save EXIF data"));
	gtk_widget_show (check);
	gtk_box_pack_start (GTK_BOX (hbox), check, TRUE, TRUE, 0);
	gtk_tooltips_set_tip (tooltips, check, _("EXIF data will be "
		"saved if this is checked"), NULL);
	save->priv->toggle_exif = GTK_TOGGLE_BUTTON (check);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (main_vbox),
			    hbox, TRUE, TRUE, 0);

	label = gtk_label_new (_("Open image(s) with: "));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	entry = gtk_entry_new ();
	gtk_widget_show (entry);
	gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);
	gtk_tooltips_set_tip (tooltips, entry, _("Type in the name of the "
			      "program you want to run, leave blank "
			      "for none"), NULL);
	save->priv->program = GTK_ENTRY (entry);

	/* Filenames provided by camera */
	check = gtk_check_button_new_with_label (_("Use filename(s) provided by the camera"));
	gtk_widget_show (check);
	gtk_box_pack_start (GTK_BOX (main_vbox),
			    check, TRUE, TRUE, 0);
	gtk_tooltips_set_tip (tooltips, check, _("Choose whether to use the "
			      "filename provided by the camera"), NULL);
	save->priv->toggle_filename_camera = GTK_TOGGLE_BUTTON (check);
	gtk_toggle_button_set_active (save->priv->toggle_filename_camera, TRUE);
	g_signal_connect (G_OBJECT (check), "toggled",
			  G_CALLBACK (on_filename_camera_toggled), save);

	hbox = gtk_hbox_new (FALSE, 5);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (main_vbox),
			    hbox, TRUE, TRUE, 0);

	save->priv->hbox_prefix = hbox;

	label = gtk_label_new (_("Filename prefix: "));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	save->priv->prefix_entry = gtk_entry_new ();
	gtk_widget_show (save->priv->prefix_entry);
	gtk_box_pack_start (GTK_BOX (hbox), save->priv->prefix_entry,
			    TRUE, TRUE, 0);

	label = gtk_label_new (_("Start numbering with: "));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	a = gtk_adjustment_new (1.0, 1.0, 10000.0, 1.0, 10.0, 10.0);
	save->priv->spin_entry = gtk_spin_button_new (GTK_ADJUSTMENT (a),
				 1.0, 0);
	gtk_widget_show (save->priv->spin_entry);
	gtk_box_pack_start (GTK_BOX (hbox), save->priv->spin_entry,
			    FALSE, FALSE, 0);

	load_save_settings(save);

	/* Remember the main window (if given) */
	save->priv->main_window = main_window ? main_window : GTK_WINDOW (save);

	return (GTK_WIDGET (save));
}

void
gtkam_save_add (GtkamSave *save, GtkamCamera *camera,
		const gchar *folder, const gchar *name)
{
	GtkamSaveData *data;
	gchar *title;

	g_return_if_fail (GTKAM_IS_SAVE (save));
	g_return_if_fail (GTKAM_IS_CAMERA (camera));
	g_return_if_fail (folder != NULL);
	g_return_if_fail (name != NULL);

	data = g_new0 (GtkamSaveData, 1);
	data->camera = camera;
	g_object_ref (G_OBJECT (camera));
	data->folder = g_strdup (folder);
	data->name = g_strdup (name);
	save->priv->data = g_slist_append (save->priv->data, data);

	switch (g_slist_length (save->priv->data)) {
	case 1:

		/* First case: We have only one file. */
		title = g_strdup_printf (_("Save '%s'..."), name);
		gtk_window_set_title (GTK_WINDOW (save), title);
		g_free (title);

		gtk_widget_hide (save->priv->toggle_filename_camera);
	
		gtk_widget_hide (save->priv->hbox_prefix);
		gtk_widget_set_sensitive (save->priv->spin_entry, FALSE);
		gtk_widget_set_sensitive (save->priv->prefix_entry, FALSE);

		gtk_file_chooser_set_action(GTK_FILE_CHOOSER (save),
				GTK_FILE_CHOOSER_ACTION_SAVE);

		gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (save), name);

		break;

	default:

		/* Second case: We have multiple files to save. */
		gtk_window_set_title (GTK_WINDOW (save), _("Save photos..."));

		gtk_widget_show (save->priv->toggle_filename_camera);

		gtk_widget_show (save->priv->hbox_prefix);
		gtk_widget_set_sensitive (save->priv->spin_entry,
				!save->priv->toggle_filename_camera->active);
		gtk_widget_set_sensitive (save->priv->prefix_entry,
				!save->priv->toggle_filename_camera->active);
		gtk_file_chooser_set_action(GTK_FILE_CHOOSER (save),
				GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);

		break;
	}
}
