/*  $Id$ */

/* 
 * Philips Digital Camera configuration gui
 *
 * Copyright (c) 1999 Bob Paauwe
 *
 * Impliments a camera configuration dialog box using the GTK
 * widget library. It relies on the Philips Digital Camera library
 * for all camera control.
 *
 * This has been tested with the Philips ESP80SXG digital camera
 * but should work with the Ricoh RDC-4300 and maybe thr RDC-4200.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <time.h>
#include "../src/gphoto.h"
#include "../src/util.h"

/* prototypes for io library calls */
#include "philips.h"

extern char *Philips_models[];
extern long cameraid; /* this should be global or returned */
void philips_cfg_page1 ();
void philips_cfg_page2 ();
void philips_cfg_page3 ();
void philips_cfg_page4 ();

struct P_CONFIG_CONTROLS {
	GtkWidget	*rm_images;
	GtkWidget	*rm_multi;
	GtkWidget	*rm_images_sound;
	GtkWidget	*rm_sound;
	GtkWidget	*rm_character;
	GtkWidget	*rm_character_sound;

	GtkWidget	*fl_on;
	GtkWidget	*fl_off;
	GtkWidget	*fl_auto;
	GtkWidget	*fl_synchro;
	GtkWidget	*fl_r_auto;
	GtkWidget	*fl_r_on;
	GtkWidget	*fl_r_synchro;

	GtkWidget	*res_640;
	GtkWidget	*res_1280;
	GtkWidget	*res_900;
	GtkWidget	*res_1800;

	GtkWidget	*q_fine;
	GtkWidget	*q_normal;
	GtkWidget	*q_economy;

	GtkWidget	*wb_outdoor;
	GtkWidget	*wb_flourescent;
	GtkWidget	*wb_incandescent;
	GtkWidget	*wb_auto;
	GtkWidget	*wb_bw;
	GtkWidget	*wb_sepia;
	GtkWidget	*wb_overcast;

	GtkWidget	*m_on;
	GtkWidget	*m_off;

	GtkWidget	*exp_auto;
	GtkWidget	*exp_manual;

	GtkWidget	*zoom;

	GtkWidget	*st_picts;
	GtkWidget	*st_memory;
	GtkWidget	*st_available;
	GtkWidget	*st_time;
	GtkWidget	*st_copyright;

	};


/*  philips_cfg_separator
 *
 *  create a horizontal or vertical separator widget
 *
 *  NOTE: This code never frees the memory allocated to a separator
 *        is that bad?
 */

void philips_cfg_separator ( GtkWidget *table, int horv, gint left, gint right, gint top, gint bottom )
{

	GtkWidget	*separator;

	if ( horv == 0 ) 		/* horizontal */
		separator = gtk_hseparator_new();
	else
		separator = gtk_vseparator_new();

	gtk_widget_show ( separator );
	gtk_table_attach_defaults ( GTK_TABLE(table), separator, left, right, top, bottom );	
}


/*  philips_cfg_label
 *
 *  create a static label widget
 *
 *  NOTE: This code never frees the memory allocated to a label
 *        is that bad?
 */

void philips_cfg_label ( char *label, GtkWidget *table, gint left, gint right, gint top, gint bottom )
{
	
	GtkWidget	*static_label;

	static_label = gtk_label_new ( label );
	gtk_misc_set_alignment ( GTK_MISC(static_label), 0, 0 );
	gtk_widget_show ( static_label );
	gtk_table_attach_defaults ( GTK_TABLE(table), static_label, left, right, top, bottom );	
}


/*  philips_cfg_new_label
 *
 *  create a dynamic label widget
 *
 *  NOTE: This code never frees the memory allocated to a label
 *        is that bad?
 */

GtkWidget *philips_cfg_new_label ( char *label, GtkWidget *table, gint left, gint right, gint top, gint bottom )
{
	
	GtkWidget	*label_w;

	label_w = gtk_label_new ( label );
	gtk_misc_set_alignment ( GTK_MISC(label_w), 0, 0 );
	gtk_widget_show ( label_w );
	gtk_table_attach_defaults ( GTK_TABLE(table), label_w, left, right, top, bottom );	
	return ( label_w );
}


/*  philips_cfg_radio
 *
 *  create a radio button widget. Return a pointer to the newly
 *  created widget after displaying it.
 */

GtkWidget *philips_cfg_radio (  )
{
	return ( NULL );
}
	

/*  philips_configure
 *
 *  Bring up the Philips configuration dialog box. This should
 *  allow most of the camera options to be changed.
 */

int philips_configure () {


	struct	P_CONFIG_CONTROLS	controls;
	PhilipsCfgInfo	*pcfginfo;
	int			error;
	GtkWidget	*dialog;
	GtkWidget	*notebook;
	GtkWidget	*button;

	char		*info, title[128];

	/* initialize camera and grab configuration information */
	
	if (philips_open_camera() == 0) {
		error_dialog ( "Could not open camera." );
		return 0;
		}

	if ( (pcfginfo = philips_getcfginfo ( &error )) == NULL ) {
		error_dialog ( "Can't get camera configuration." );
		philips_close_camera();
		return ( 0 );
		}
	philips_close_camera();

	update_progress(12);

	sprintf ( title, "Configure Camera %s", philips_model(cameraid) );
	info = (char *)malloc(2048);

	/* create a new dialog box */
	dialog = gtk_dialog_new();
	gtk_window_set_title (GTK_WINDOW(dialog), title);
	gtk_container_border_width(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), 10);

	/* create a new notebook, place the position of the tabs */
	notebook = gtk_notebook_new ();
	gtk_notebook_set_tab_pos ( GTK_NOTEBOOK(notebook), GTK_POS_TOP );
	gtk_widget_show ( notebook );
	gtk_container_add ( GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), notebook );

	/* add a page to the notebook */
	philips_cfg_page1 ( notebook, &controls, pcfginfo );
	update_progress(25);
	philips_cfg_page2 ( notebook, &controls, pcfginfo );
	update_progress(50);
	philips_cfg_page3 ( notebook, &controls, pcfginfo );
	update_progress(75);
	philips_cfg_page4 ( notebook, &controls, pcfginfo );


	/* create an OK button */
	button = gtk_button_new_with_label ( " OK " );
	gtk_signal_connect_object ( GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(gtk_widget_hide), GTK_OBJECT(dialog) );
		    
	gtk_box_pack_end ( GTK_BOX(GTK_DIALOG(dialog)->action_area), button, TRUE, FALSE, 0 );
	gtk_widget_show ( button );

	/* create a Cancel button */
	button = gtk_button_new_with_label ( " Cancel " );
	gtk_signal_connect_object ( GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(gtk_widget_hide), GTK_OBJECT(dialog) );
	gtk_box_pack_end ( GTK_BOX(GTK_DIALOG(dialog)->action_area), button, TRUE, FALSE, 0 );
	gtk_widget_show ( button );

	update_progress(100);

	gtk_widget_show ( dialog );
	update_status ( "Done." );
	update_progress ( 0 );

	while (GTK_WIDGET_VISIBLE(dialog))
		gtk_main_iteration();

/*
	if (strcmp("Cancel", (char*)gtk_object_get_data(GTK_OBJECT(dialog), "button"))==0) {
		printf ( "Cancel button pressed, return 1\n" );
        return 1;
		}
*/

	printf ( "Done with config, return 1\n" );
	return 1;
}

void philips_cfg_page1 ( GtkWidget *notebook, struct P_CONFIG_CONTROLS *controls, PhilipsCfgInfo  *pcfginfo )
{
	GtkWidget	*frame;
	GtkWidget	*label;
	GtkWidget	*table;
	char		tmpstr[1024];

	frame = gtk_frame_new ( "" );
	gtk_container_border_width ( GTK_CONTAINER(frame), 10 );
	gtk_widget_set_usize ( frame, 180, 150 );
	gtk_widget_show ( frame );

	table = gtk_table_new ( 5, 2, TRUE );
	gtk_container_add ( GTK_CONTAINER(frame), table );

	philips_cfg_label ( "Number of pictures:", table, 0, 1, 0, 1 );
	sprintf ( tmpstr, "%ld", pcfginfo->picts );
	controls->st_picts = philips_cfg_new_label ( tmpstr, table, 1, 2, 0, 1 );

	philips_cfg_label ( "Available Memory:", table, 0, 1, 1, 2 );
	sprintf ( tmpstr, "%d bytes", pcfginfo->a_memory );
	controls->st_available = philips_cfg_new_label ( tmpstr, table, 1, 2, 1, 2 );

	philips_cfg_label ( "Total Memory:", table, 0, 1, 2, 3 );
	sprintf ( tmpstr, "%d bytes", pcfginfo->memory );
	controls->st_memory = philips_cfg_new_label ( tmpstr, table, 1, 2, 2, 3 );

	philips_cfg_label ( "Camera Time/Date:", table, 0, 1, 3, 4 );
	sprintf ( tmpstr, "%s", ctime(&(pcfginfo->date)) );
	controls->st_time = philips_cfg_new_label ( tmpstr, table, 1, 2, 3, 4 );

	philips_cfg_label ( "Picture Copyright:", table, 0, 1, 4, 5 );
	sprintf ( tmpstr, "%-21.21s", pcfginfo->copyright );
	controls->st_copyright = philips_cfg_new_label ( tmpstr, table, 1, 2, 4, 5 );

	gtk_widget_show ( table );

	label = gtk_label_new ( "Camera Status" );
	gtk_notebook_append_page ( GTK_NOTEBOOK(notebook), frame, label );
}


void philips_cfg_page2 ( GtkWidget *notebook, struct P_CONFIG_CONTROLS *controls, PhilipsCfgInfo  *pcfginfo )
{
	GtkWidget	*frame;
	GtkWidget	*label;
	GtkWidget	*box, *vbox;
	GSList		*group;

	vbox = gtk_hbox_new ( FALSE, 5 );

	frame = gtk_frame_new ( "Resolution" );
	gtk_container_border_width ( GTK_CONTAINER(frame), 10 );
	gtk_widget_set_usize ( frame, 130, 75 );
	gtk_widget_show ( frame );

	box = gtk_vbox_new ( FALSE, 5 );
	if ( cameraid != 5000 ) {
		controls->res_640 = gtk_radio_button_new_with_label ( NULL, "640 x 480" );
		gtk_box_pack_start ( GTK_BOX(box), controls->res_640, FALSE, FALSE, 0 );
		if ( pcfginfo->resolution == 1 )
			gtk_toggle_button_set_state ( GTK_TOGGLE_BUTTON(controls->res_640), TRUE );
		gtk_widget_show ( controls->res_640 );
		group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->res_640) );
		}
	if ( cameraid >= 4000 && cameraid != 5000 ) {
		controls->res_1280 = gtk_radio_button_new_with_label ( group, "1280 x 960" );
		gtk_box_pack_start ( GTK_BOX(box), controls->res_1280, FALSE, FALSE, 0 );
		if ( pcfginfo->resolution == 4 )
			gtk_toggle_button_set_state ( GTK_TOGGLE_BUTTON(controls->res_640), TRUE );
		gtk_widget_show ( controls->res_1280 );
		group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->res_1280) );
		}
	if ( cameraid == 5000 ) {
		controls->res_900 = gtk_radio_button_new_with_label ( NULL, "900 x 600" );
		gtk_box_pack_start ( GTK_BOX(box), controls->res_900, FALSE, FALSE, 0 );
		if ( pcfginfo->resolution == 5 )
			gtk_toggle_button_set_state ( GTK_TOGGLE_BUTTON(controls->res_900), TRUE );
		gtk_widget_show ( controls->res_900 );
		group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->res_900) );
		controls->res_1800 = gtk_radio_button_new_with_label ( NULL, "1800 x 900" );
		gtk_box_pack_start ( GTK_BOX(box), controls->res_1800, FALSE, FALSE, 0 );
		if ( pcfginfo->resolution == 6 )
			gtk_toggle_button_set_state ( GTK_TOGGLE_BUTTON(controls->res_1800), TRUE );
		gtk_widget_show ( controls->res_1800 );
		group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->res_1800) );
		}
	gtk_container_add ( GTK_CONTAINER(frame), box );
	gtk_widget_show ( box );
	gtk_box_pack_start ( GTK_BOX(vbox), frame, FALSE, FALSE, 0 );
	gtk_widget_show ( vbox );

	frame = gtk_frame_new ( "Quality" );
	gtk_container_border_width ( GTK_CONTAINER(frame), 10 );
	gtk_widget_set_usize ( frame, 130, 75 );
	gtk_widget_show ( frame );

	box = gtk_vbox_new ( FALSE, 5 );
	controls->q_fine = gtk_radio_button_new_with_label ( NULL, "Fine" );
	gtk_box_pack_start ( GTK_BOX(box), controls->q_fine, FALSE, FALSE, 0 );
	gtk_widget_show ( controls->q_fine );
	group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->q_fine) );
	controls->q_normal = gtk_radio_button_new_with_label ( group, "Normal" );
	gtk_box_pack_start ( GTK_BOX(box), controls->q_normal, FALSE, FALSE, 0 );
	gtk_widget_show ( controls->q_normal );
	group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->q_normal) );
	controls->q_economy = gtk_radio_button_new_with_label ( group, "Economy" );
	gtk_box_pack_start ( GTK_BOX(box), controls->q_economy, FALSE, FALSE, 0 );
	gtk_widget_show ( controls->q_economy );
	group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->q_economy) );
	gtk_container_add ( GTK_CONTAINER(frame), box );
	gtk_widget_show ( box );
	gtk_box_pack_start ( GTK_BOX(vbox), frame, FALSE, FALSE, 0 );

	label = gtk_label_new ( "Image Quality" );
	gtk_notebook_append_page ( GTK_NOTEBOOK(notebook), vbox, label );
}


void philips_cfg_page3 ( GtkWidget *notebook, struct P_CONFIG_CONTROLS *controls, PhilipsCfgInfo  *pcfginfo )
{
	GtkWidget	*frame;
	GtkWidget	*label;
	GtkWidget	*box, *vbox, *v2box;
	GtkWidget	*scale;
	GSList		*group;
	GtkObject	*adj;

	vbox = gtk_vbox_new ( FALSE, 5 );

	frame = gtk_frame_new ( "White Balance" );
	gtk_container_border_width ( GTK_CONTAINER(frame), 10 );
	gtk_widget_set_usize ( frame, 100, 75 );
	gtk_widget_show ( frame );

	box = gtk_hbox_new ( FALSE, 5 );
	controls->wb_auto = gtk_radio_button_new_with_label ( NULL, "Automatic" );
	gtk_box_pack_start ( GTK_BOX(box), controls->wb_auto, FALSE, FALSE, 0 );
	gtk_widget_show ( controls->wb_auto );
	group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->wb_auto) );
	controls->wb_outdoor = gtk_radio_button_new_with_label ( group, "Outdoors" );
	gtk_box_pack_start ( GTK_BOX(box), controls->wb_outdoor, FALSE, FALSE, 0 );
	gtk_widget_show ( controls->wb_outdoor );
	group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->wb_outdoor) );

	controls->wb_flourescent = gtk_radio_button_new_with_label ( group, "Fluorescent" );
	gtk_box_pack_start ( GTK_BOX(box), controls->wb_flourescent, FALSE, FALSE, 0 );
	gtk_widget_show ( controls->wb_flourescent );
	group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->wb_flourescent) );

	controls->wb_incandescent = gtk_radio_button_new_with_label ( group, "Incandescent" );
	gtk_box_pack_start ( GTK_BOX(box), controls->wb_incandescent, FALSE, FALSE, 0 );
	gtk_widget_show ( controls->wb_incandescent );
	group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->wb_incandescent) );
	if ( (cameraid == 5000) || (cameraid == 4200) ) {
		controls->wb_bw = gtk_radio_button_new_with_label ( group, "Black & White" );
		gtk_box_pack_start ( GTK_BOX(box), controls->wb_bw, FALSE, FALSE, 0 );
		gtk_widget_show ( controls->wb_bw );
		group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->wb_bw) );
		controls->wb_sepia = gtk_radio_button_new_with_label ( group, "Sepia" );
		gtk_box_pack_start ( GTK_BOX(box), controls->wb_sepia, FALSE, FALSE, 0 );
		gtk_widget_show ( controls->wb_sepia );
		group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->wb_sepia) );
		}
	if ( cameraid == 5000 ) {
		controls->wb_overcast = gtk_radio_button_new_with_label ( group, "Overcast" );
		gtk_box_pack_start ( GTK_BOX(box), controls->wb_overcast, FALSE, FALSE, 0 );
		gtk_widget_show ( controls->wb_overcast );
		group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->wb_overcast) );
		}

	gtk_container_add ( GTK_CONTAINER(frame), box );
	gtk_widget_show ( box );
	gtk_box_pack_start ( GTK_BOX(vbox), frame, FALSE, FALSE, 0 );
	gtk_widget_show ( vbox );

	frame = gtk_frame_new ( "Exposure Compensation" );
	gtk_container_border_width ( GTK_CONTAINER(frame), 10 );
	gtk_widget_set_usize ( frame, 390, 150 );
	gtk_widget_show ( frame );

	v2box = gtk_vbox_new ( FALSE, 5 );

	box = gtk_hbox_new ( FALSE, 5 );
	controls->exp_auto = gtk_radio_button_new_with_label ( NULL, "Automatic" );
	gtk_box_pack_start ( GTK_BOX(box), controls->exp_auto, FALSE, FALSE, 0 );
	gtk_widget_show ( controls->exp_auto );
	group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->exp_auto) );
	controls->exp_manual = gtk_radio_button_new_with_label ( group, "Manual" );
	gtk_box_pack_start ( GTK_BOX(box), controls->exp_manual, FALSE, FALSE, 0 );
	gtk_widget_show ( controls->exp_manual );
	group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->exp_manual) );
	gtk_box_pack_start ( GTK_BOX(v2box), box, FALSE, FALSE, 0 );

	gtk_widget_show ( box );

	adj = gtk_adjustment_new ( 0.0, -2.0, 2.0, 0.5, 1.0, 0.0 );
	scale = gtk_hscale_new ( GTK_ADJUSTMENT(adj) );
	gtk_box_pack_start ( GTK_BOX(v2box), scale, FALSE, FALSE, 0 );
	gtk_widget_show ( scale );

	gtk_container_add ( GTK_CONTAINER(frame), v2box );
	gtk_widget_show ( v2box );

	gtk_box_pack_start ( GTK_BOX(vbox), frame, FALSE, FALSE, 0 );

	label = gtk_label_new ( "Exposure Control" );
	gtk_notebook_append_page ( GTK_NOTEBOOK(notebook), vbox, label );
}


void philips_cfg_page4 ( GtkWidget *notebook, struct P_CONFIG_CONTROLS *controls, PhilipsCfgInfo  *pcfginfo )
{
	GtkWidget	*frame;
	GtkWidget	*label;
	GtkWidget	*box, *vbox, *zbox, *fmbox, *ybox, *box1;
	GtkWidget	*scale;
	GtkObject	*adj;
	GSList		*group;

	vbox = gtk_hbox_new ( FALSE, 5 );

	/*******************/
	/* Recording frame */
	/*******************/
	frame = gtk_frame_new ( "Recording Mode" );
	gtk_container_border_width ( GTK_CONTAINER(frame), 10 );
	gtk_widget_set_usize ( frame, 170, 135 );
	gtk_widget_show ( frame );

	box = gtk_vbox_new ( FALSE, 5 );
	controls->rm_images = gtk_radio_button_new_with_label ( NULL, "Images" );
	gtk_box_pack_start ( GTK_BOX(box), controls->rm_images, FALSE, FALSE, 0 );
	gtk_widget_show ( controls->rm_images );
	group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->rm_images) );
	if ( cameraid == 5000 ) {
		controls->rm_multi = gtk_radio_button_new_with_label ( group, "Multi-Shot" );
		gtk_box_pack_start ( GTK_BOX(box), controls->rm_multi, FALSE, FALSE, 0 );
		gtk_widget_show ( controls->rm_multi );
		group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->rm_multi) );
		}
	if ( (cameraid != 4200) && (cameraid != 5000) ) {
		controls->rm_images_sound = gtk_radio_button_new_with_label ( group, "Images+sound" );
		gtk_box_pack_start ( GTK_BOX(box), controls->rm_images_sound, FALSE, FALSE, 0 );
		gtk_widget_show ( controls->rm_images_sound );
		group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->rm_images_sound) );
		controls->rm_sound = gtk_radio_button_new_with_label ( group, "Sound" );
		gtk_box_pack_start ( GTK_BOX(box), controls->rm_sound, FALSE, FALSE, 0 );
		gtk_widget_show ( controls->rm_sound );
		group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->rm_sound) );
		}
	controls->rm_character = gtk_radio_button_new_with_label ( group, "Character" );
	gtk_box_pack_start ( GTK_BOX(box), controls->rm_character, FALSE, FALSE, 0 );
	gtk_widget_show ( controls->rm_character );
	group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->rm_character) );
	if ( (cameraid != 4200) && (cameraid != 5000) ) {
		controls->rm_character_sound = gtk_radio_button_new_with_label ( group, "Character+sound" );
		gtk_box_pack_start ( GTK_BOX(box), controls->rm_character_sound, FALSE, FALSE, 0 );
		gtk_widget_show ( controls->rm_character_sound );
		group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->rm_character_sound) );
		}
	gtk_container_add ( GTK_CONTAINER(frame), box );
	gtk_widget_show ( box );
	gtk_box_pack_start ( GTK_BOX(vbox), frame, FALSE, FALSE, 0 );
	gtk_widget_show ( vbox );

	zbox = gtk_vbox_new ( FALSE, 5 );  /* box up flash/macro and zoom */
	fmbox = gtk_hbox_new ( FALSE, 5 ); /* box up flash and macro */

	/****************/
	/*  Flash frame */
	/****************/
	frame = gtk_frame_new ( "Flash Mode" );
	gtk_container_border_width ( GTK_CONTAINER(frame), 10 );
	if ( cameraid != 5000 ) {
		gtk_widget_set_usize ( frame, 130, 145 );
		}
	else {
		gtk_widget_set_usize ( frame, 260, 145 );
		}
	gtk_widget_show ( frame );

	ybox = gtk_hbox_new ( FALSE, 5 );
	box = gtk_vbox_new ( FALSE, 5 );
	controls->fl_auto = gtk_radio_button_new_with_label ( NULL, "Auto" );
	gtk_box_pack_start ( GTK_BOX(box), controls->fl_auto, FALSE, FALSE, 0 );
	gtk_widget_show ( controls->fl_auto );
	group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->fl_auto) );
	controls->fl_on = gtk_radio_button_new_with_label ( group, "On" );
	gtk_box_pack_start ( GTK_BOX(box), controls->fl_on, FALSE, FALSE, 0 );
	gtk_widget_show ( controls->fl_on );
	group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->fl_on) );
	controls->fl_off = gtk_radio_button_new_with_label ( group, "Off" );
	gtk_box_pack_start ( GTK_BOX(box), controls->fl_off, FALSE, FALSE, 0 );
	gtk_widget_show ( controls->fl_off );
	group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->fl_off) );
	if ( cameraid == 5000 ) {
		controls->fl_synchro = gtk_radio_button_new_with_label ( group, "Synchronized" );
		gtk_box_pack_start ( GTK_BOX(box), controls->fl_synchro, FALSE, FALSE, 0 );
		gtk_widget_show ( controls->fl_synchro );
		group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->fl_synchro) );
		}
	gtk_box_pack_start ( GTK_BOX(ybox), box, FALSE, FALSE, 0 );
	gtk_widget_show ( box );

	if ( cameraid == 5000 ) {
		box1 = gtk_vbox_new ( FALSE, 5 );
		controls->fl_r_on = gtk_radio_button_new_with_label ( group, "On w/ RedEye" );
		gtk_box_pack_start ( GTK_BOX(box1), controls->fl_r_on, FALSE, FALSE, 0 );
		gtk_widget_show ( controls->fl_r_on );
		group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->fl_r_on) );
		controls->fl_r_auto = gtk_radio_button_new_with_label ( group, "Auto w/ RedEye" );
		gtk_box_pack_start ( GTK_BOX(box1), controls->fl_r_auto, FALSE, FALSE, 0 );
		gtk_widget_show ( controls->fl_r_auto );
		group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->fl_r_auto) );
		controls->fl_r_synchro = gtk_radio_button_new_with_label ( group, "Syncrhonzied w/ RedEye" );
		gtk_box_pack_start ( GTK_BOX(box1), controls->fl_r_synchro, FALSE, FALSE, 0 );
		gtk_widget_show ( controls->fl_r_synchro );
		group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->fl_r_synchro) );
		gtk_box_pack_start ( GTK_BOX(ybox), box1, FALSE, FALSE, 0 );
		gtk_widget_show ( box1 );
		}
	gtk_container_add ( GTK_CONTAINER(frame), ybox );
	gtk_widget_show ( ybox );

	gtk_box_pack_start ( GTK_BOX(fmbox), frame, FALSE, FALSE, 0 );
	gtk_widget_show ( fmbox );

	/***************/
	/* Macro frame */
	/***************/
	if ( cameraid != 5000 ) {
		frame = gtk_frame_new ( "Macro Mode" );
		gtk_container_border_width ( GTK_CONTAINER(frame), 10 );
		gtk_widget_set_usize ( frame, 130, 105 );
		gtk_widget_show ( frame );
	
		box = gtk_vbox_new ( FALSE, 5 );
		controls->m_on = gtk_radio_button_new_with_label ( NULL, "On" );
		gtk_box_pack_start ( GTK_BOX(box), controls->m_on, FALSE, FALSE, 0 );
		gtk_widget_show ( controls->m_on );
		group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->m_on) );
		controls->m_off = gtk_radio_button_new_with_label ( group, "Off" );
		gtk_box_pack_start ( GTK_BOX(box), controls->m_off, FALSE, FALSE, 0 );
		gtk_widget_show ( controls->m_off );
		group = gtk_radio_button_group ( GTK_RADIO_BUTTON(controls->m_off) );
		gtk_container_add ( GTK_CONTAINER(frame), box );
		gtk_widget_show ( box );
		gtk_box_pack_start ( GTK_BOX(fmbox), frame, FALSE, FALSE, 0 );
		}
	
	/* end of packing into fmbox */
	gtk_box_pack_start ( GTK_BOX(zbox), fmbox, FALSE, FALSE, 0 );
	gtk_widget_show ( zbox );

	/**************/
	/* Zoom frame */
	/**************/
	frame = gtk_frame_new ( "Zoom Level" );
	gtk_container_border_width ( GTK_CONTAINER(frame), 10 );
	gtk_widget_set_usize ( frame, 230, 75 );
	gtk_widget_show ( frame );
	adj = gtk_adjustment_new ( 0.0, 0.0, 8.0, 1.0, 1.0, 0.0 );
	scale = gtk_hscale_new ( GTK_ADJUSTMENT(adj) );
	gtk_widget_show ( scale );
	gtk_container_add ( GTK_CONTAINER(frame), scale );
	gtk_box_pack_start ( GTK_BOX(zbox), frame, FALSE, FALSE, 0 );
	/* end of packing into zbox */

	gtk_box_pack_start ( GTK_BOX(vbox), zbox, FALSE, FALSE, 0 );
	/* end of packing into vbox */


	label = gtk_label_new ( "General" );
	gtk_notebook_append_page ( GTK_NOTEBOOK(notebook), vbox, label );
}
