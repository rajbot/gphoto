/* Gphoto - free digital camera utility - http://www.gphoto.org/
 *
 * Copyright (C) 1999  The gPhoto developers  <gphoto-devel@gphoto.org>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "main.h"
#include "gphoto.h"
#include "menu.h"
#include "callbacks.h"
#include "gallery.h"
#include "live.h"
#include "developer_dialog.h"

/* Build the Menu --------------------------------------------
   ----------------------------------------------------------- */
#ifdef  GTK_HAVE_FEATURES_1_1_0
GtkAccelGroup *mainag;
#endif

void menu_select(gpointer data, guint action, GtkWidget *widget)
{
/* use this until we can adapt the other callbacks to what the item
 * factory expects for arguments.
 * just note this is temporary. :) 
 */

	switch (action) {
		case 1: /* Open pic */
			open_dialog(widget, data);
			break;
		case 2: /* Save pic */
			save_dialog(widget, data);
			break;
		case 3: /* Print pic */
			print_pic();
			break;
		case 4: /* Close pic */
			closepic();
			break;
		case 5: /* Quit */
			delete_event(widget, NULL, NULL);
			break;
		case 6: /* Rotate Clockwise */
			manip_pic("r");
			break;
		case 7: /* Rotate Counter-clockwise */
			manip_pic("l");
			break;
		case 8: /* Flip Horizontal */
			manip_pic("h");
			break;
		case 9: /* Flip Vertical */
			manip_pic("v");
			break;
		case 10: /* Scale Half */
			scale_half();
			break;
		case 11: /* Scale Double */
			scale_double();
			break;
		case 12: /* Resize */
			resize_dialog();
			break;
		case 13: /* Select All */
			select_all();
			break;
		case 14: /* Select Inverse */
			select_inverse();
			break;
		case 15: /* Select None */
			select_none();
			break;
		case 16: /* Download Thumbnails */
			getindex();
			break;
		case 17: /* Download blank */
			getindex_empty();
			break;
		case 20: /* Delete selected */
			del_dialog(0);
			break;
		case 21: /* Take picture */
			takepicture_call();
			break;
		case 22: /* Camera summary */
			summary_dialog();
			break;
		case 23: /* Port / model */
			port_dialog();
			break;
		case 24: /* Configure */
			configure_call();
			break;
		case 25: /* HTML Gallery */
			gallery_main();
			break;
		case 26: /* Live Camera */
			live_main();
			break;
		case 27: /* Authors */
			developer_dialog_create();
			break;
		case 28: /* License */
			show_license();
			break;
		case 29: /* Version */
			version_dialog();
			break;
		case 30: /* Users manual */
			usersmanual_dialog();
			break;
		case 31: /* Online Help */
			browse_help();
			break;
		case 32: /* Open Directory */
			set_camera("Browse Directory");
			makeindex(1);
			break;
		case 33: /* Online News */
			browse_news();
			break;
		case 34: /* Online Updates */
			browse_download();
			break;
		case 35: /* Online Cameras */
			browse_cameras();
			break;
		case 36: /* Online Supporting */
			browse_supporting();
			break;
		case 37: /* Online Discussion */
			browse_discussion();
			break;
		case 38: /* Online Team */
			browse_team();
			break;
		case 39: /* Online Themes */
			browse_themes();
			break;
		case 40: /* Online Links */
			browse_links();
			break;
		case 41: /* Online To Do */
			browse_todo();
			break;
		case 42: /* Online Feedback */
			browse_feedback();
			break;
		case 43: /* Color balance */
			color_dialog();
			break;
	       case 44: /* Next page */
	                next_page();
	                break;
      	        case 45: /* Previous page */
		        prev_page();
  		        break;
	        case 46: /* Index page */
		        index_page();
		        break;
	        case 47: /* Format */
		        del_dialog(1);
		        break;
	}
}

/* Oh WOW is this a lot easier :) */
/* Thanks to Eric Harlow for his book "Developing Linux Applications".
   This is a definite must-buy/read/worship for GTK development!! */

GtkItemFactoryEntry menu_items[] = {

	{"/_File",						NULL, 0,		0, "<Branch>"},
	{"/File/_Open",				                NULL, 0,	        0, "<Branch>"},
	{"/File/Open/Directory...",			"<control>o", menu_select,	32},
	{"/File/Open/File...",				"<control>f", menu_select,	1},
	{"/File/sep2",						NULL, 0,		0, "<Separator>"},
	{"/File/_Save",				                NULL, 0,	        0, "<Branch>"},
	{"/File/Save/Opened Image(s)...", 		"<control>s", menu_select, 2},
	{"/File/Save/Selected Thumbnails...",	        "<control>t", save_thumbs,	0},
	{"/File/Save/Selected Images...",              "<control>g", save_images,	19},
	{"/File/Save/Selected Thumbnails and Images...",      	NULL, save_both,	0},
	{"/File/sep2",						NULL, 0,		0, "<Separator>"},
	{"/File/_Export",					NULL, 0,		0, "<Branch>"},
	{"/File/Export/HTML _Gallery",			"<control>m", menu_select, 25},
	{"/File/sep1",						NULL, 0,		0, "<Separator>"},
	{"/File/_Print",				"<control>p", menu_select,	3},
	{"/File/sep2",						NULL, 0,		0, "<Separator>"},
	{"/File/_Close",				"<control>W", menu_select,	4},
	{"/File/_Quit",					"<control>q", menu_select,	5},
	{"/_Camera",					 	NULL, 0, 		0, "<Branch>"},
	{"/Camera/_Summary",				NULL, menu_select, 22},
	{"/Camera/Download _Index",			NULL, 0,0, "<Branch>"},
	{"/Camera/Download Index/_Thumbnails",		"<control>i", menu_select,	16},
	{"/Camera/Download Index/_No Thumbnails",		"<control>e", menu_select,	17},
	{"/Camera/Download _Selected",				NULL, 0,		0, "<Branch>"},
	{"/Camera/Download Selected/_Images",			NULL, 0,		0, "<Branch>"},
	{"/Camera/Download Selected/Images/_Open in window",		NULL, open_images,	18},
	{"/Camera/Download Selected/Images/_Save to disk...","<control>g", save_images,	19},
	{"/Camera/Download Selected/_Thumbnails",			NULL, 0,		0, "<Branch>"},
	{"/Camera/Download Selected/Thumbnails/_Open in window",	NULL, open_thumbs,	0},
	{"/Camera/Download Selected/Thumbnails/_Save to disk...",	NULL, save_thumbs,	0},
	{"/Camera/Download Selected/_Both",				NULL, 0,		0, "<Branch>"},
	{"/Camera/Download Selected/Both/_Open in window",		NULL, open_both,	0},
	{"/Camera/Download Selected/Both/_Save to disk...",		NULL, save_both,	0},
	{"/Camera/_Delete",						NULL, 0, 0, "<Branch>"},
	{"/Camera/Delete/Selected Images",			NULL, menu_select,	20},
	{"/Camera/Delete/All Images",			        NULL, menu_select,	47},
	{"/Camera/_Live Preview!",				NULL, menu_select,	26},
	{"/Camera/_Take Picture",				NULL, menu_select,	21},

	{"/_Edit",						NULL, 0,		0, "<Branch>"},
	{"/Edit/_Size",					        NULL, 0,		0, "<Branch>"},
	{"/Edit/Size/Scale _Half",				NULL, menu_select,	10},
	{"/Edit/Size/Scale _Double",				NULL, menu_select,	11},
	{"/Edit/Size/_Scale",					NULL, menu_select,	12},
	{"/Edit/sep2",						NULL, 0,		0, "<Separator>"},
	{"/Edit/_Color Balance",				NULL, menu_select,	43},

	{"/_Go",						NULL, 0,                0,"<Branch>"},
	{"/Go/_Index page",				  "<alt>i", menu_select,	46},
	{"/Go/_Next page",				  "<alt>n", menu_select,	44},
	{"/Go/_Previous page",			          "<alt>p", menu_select,	45},
	{"/_Select",						NULL, 0,                0,"<Branch>"},
       	{"/Select/_All images",				  "<shift>a", menu_select,	13},
	{"/Select/_Inverse",			          "<shift>i", menu_select,	14},
	{"/Select/_None",			 	  "<shift>n", menu_select,	15},

	{"/_Orientation",					NULL, 0,		0, "<Branch>"},
	{"/Orientation/Rotate Clockwise", 			NULL, menu_select,	6},
	{"/Orientation/Rotate Counter-Clockwise", 		NULL, menu_select,	7},
	{"/Orientation/sep2",					NULL, 0,		0, "<Separator>"},
	{"/Orientation/Flip Horizontal", 			NULL, menu_select,	8},
	{"/Orientation/Flip Vertical", 			        NULL, menu_select,	9},
	{"/Configu_re",						NULL, 0,		0, "<Branch>"},
	{"/Configure/_Select Port-Camera Model",		NULL, menu_select,	23},
	{"/Configure/_Configure Camera...",			NULL, menu_select,	24},
	{"/_Help",						NULL, 0,		0, "<LastBranch>"},
	{"/Help/_Authors",					NULL, menu_select,	27},
	{"/Help/_License",					NULL, menu_select,	28},
	{"/Help/_Version",					NULL, menu_select,	29},
	{"/Help/_User's Manual",			"<control>h", menu_select,	30},
        {"/Help/sep4",				                NULL, 0, 		0, "<Separator>"},
	{"/Help/_www.gphoto.org",				NULL, 0,		0, "<Branch>"},
	{"/Help/www.gphoto.org/Hel_p",				NULL, menu_select,	31},
	{"/Help/www.gphoto.org/Ne_ws",			        NULL, menu_select,	33},
	{"/Help/www.gphoto.org/Updates_",			NULL, menu_select,	34},
	{"/Help/www.gphoto.org/Cameras_",			NULL, menu_select,	35},
	{"/Help/www.gphoto.org/Supporting_",			NULL, menu_select,	36},
	{"/Help/www.gphoto.org/Di_scussion",		        NULL, menu_select,	37},
	{"/Help/www.gphoto.org/Devel. Team_",		        NULL, menu_select,	38},
	{"/Help/www.gphoto.org/Themes_",		        NULL, menu_select,	39},
	{"/Help/www.gphoto.org/Lin_ks",			        NULL, menu_select,	40},
	{"/Help/www.gphoto.org/To Do_",			        NULL, menu_select,	41},
	{"/Help/www.gphoto.org/Feed_back",		        NULL, menu_select,	42},
};

void create_menu(GtkWidget *menu_bar)
{
	GtkItemFactory *item_factory;
	int menu_items_size = sizeof(menu_items) / sizeof (menu_items[0]);

	mainag = gtk_accel_group_new();

	item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<gp>", mainag);
	gtk_item_factory_create_items(item_factory, menu_items_size, menu_items, NULL);
	gtk_box_pack_start (GTK_BOX(menu_bar), gtk_item_factory_get_widget(item_factory, "<gp>"),
			FALSE, FALSE, 0);
}
