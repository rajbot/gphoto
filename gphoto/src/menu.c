/* gPhoto - free digital camera utility - http://www.gphoto.org/
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
GtkAccelGroup*  mainag;
#endif

void menu_select (gpointer data, guint action, GtkWidget *widget) {

/* use this until we can adapt the other callbacks to what the item
 * factory expects for arguments.
 * just note this is temporary. :) 
 */

	switch (action) {
		case 1: /* Open pic */
			filedialog("o");
			break;
		case 2: /* Save pic */
			filedialog("s");
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
		case 16: /* Get Thumbnails */
			getindex();
			break;
		case 17: /* Get blank */
			getindex_empty();
			break;
		case 20: /* Delete selected */
			del_dialog();
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
			strcpy(camera_model, "Browse Directory");
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
		default:
	}
}

/* Oh WOW is this a lot easier :) */

GtkItemFactoryEntry menu_items[] = {
	{"/_File",						NULL, 0,		0,	"<Branch>"},
	{"/File/_Open...",				"<control>o", menu_select,	1},
	{"/File/Open _Directory...",				NULL, menu_select,	32},
	{"/File/_Save Image...",			"<control>s", menu_select,	2},
	{"/File/sep1",						NULL, 0,		0,	"<Separator>"},
	{"/File/_Print",				"<control>p", menu_select,	3},
	{"/File/sep2",						NULL, 0,		0,	"<Separator>"},
	{"/File/_Close",					NULL, menu_select,	4},
	{"/File/_Quit",					"<control>q", menu_select,	5},

	{"/_Edit",						NULL, 0,		0,	"<Branch>"},
	{"/Edit/Image _Orientation",				NULL, 0,		0,	"<Branch>"},
	{"/Edit/Image Orientation/Rotate Clockwise", 		NULL, menu_select,	6},
	{"/Edit/Image Orientation/Rotate Counter-Clockwise", 	NULL, menu_select,	7},
	{"/Edit/Image Orientation/Flip Horizontal", 		NULL, menu_select,	8},
	{"/Edit/Image Orientation/Flip Vertical", 		NULL, menu_select,	9},
	{"/Edit/Image _Dimension",				NULL, 0,		0,	"<Branch>"},
	{"/Edit/Image Dimension/Scale _Half",			NULL, menu_select,	10},
	{"/Edit/Image Dimension/Scale _Double",			NULL, menu_select,	11},
	{"/Edit/Image Dimension/_Scale",			NULL, menu_select,	12},
	{"/Edit/sep3",						NULL, 0, 		0,	"<Separator>"},
	{"/Edit/Select _All",				  "<shift>a", menu_select,	13},
	{"/Edit/Select _Inverse",			  "<shift>i", menu_select,	14},
	{"/Edit/Select _None",			 	  "<shift>n", menu_select,	15},


	{"/_Camera",					 	NULL, 0, 		0,	"<Branch>"},
	{"/Camera/Get _Index",			        	NULL, 0,		0,	"<Branch>"},
	{"/Camera/Get Index/_Thumbnails",		"<control>i", menu_select,	16},
	{"/Camera/Get Index/_No Thumbnails",		"<control>e", menu_select,	17},
	{"/Camera/Get _Selected",				NULL, 0,		0, 	"<Branch>"},
	{"/Camera/Get Selected/_Images",			NULL, 0,		0,	"<Branch>"},
	{"/Camera/Get Selected/Images/_Open in window",		NULL, open_images,	18},
	{"/Camera/Get Selected/Images/_Save to disk...","<control>g", save_images,	19},
	{"/Camera/Get Selected/_Thumbnails",			NULL, 0,		0,	"<Branch>"},
	{"/Camera/Get Selected/Thumbnails/_Open in window",	NULL, open_thumbs,	0},
	{"/Camera/Get Selected/Thumbnails/_Save to disk...",	NULL, save_thumbs,	0},
	{"/Camera/Get Selected/_Both",				NULL, 0,		0,	"<Branch>"},
	{"/Camera/Get Selected/Both/_Open in window",		NULL, open_both,	0},
	{"/Camera/Get Selected/Both/_Save to disk...",		NULL, save_both,	0},
	{"/Camera/sep3",					NULL, 0, 		0,	"<Separator>"},
	{"/Camera/_Delete Selected Images",			NULL, menu_select,	20},
	{"/Camera/_Take Picture",				NULL, menu_select,	21},
	{"/Camera/_Camera Summary",				NULL, menu_select,	22},
	{"/C_onfigure",						NULL, 0,		0,	"<Branch>"},
	{"/Configure/_Select Port-Camera Model",		NULL, menu_select,	23},
	{"/Configure/_Configure Camera...",			NULL, menu_select,	24},

	{"/_Plugins",						NULL, 0,		0,	"<Branch>"},
	{"/Plugins/HTML _Gallery",				NULL, menu_select,	25},
	{"/Plugins/_Live Camera!",				NULL, menu_select,	26},

	{"/_Help",						NULL, 0,		0,	"<LastBranch>"},
	{"/Help/_Authors",					NULL, menu_select,	27},
	{"/Help/_License",					NULL, menu_select,	28},
	{"/Help/_Version",					NULL, menu_select,	29},
	{"/Help/_User's Manual",				NULL, menu_select,	30},
	{"/Help/sep4",					NULL, 0, 		0,	"<Separator>"},
	{"/Help/_www.gphoto.org",				NULL, 0,		0, 	"<Branch>"},
	{"/Help/www.gphoto.org/Hel_p",				        NULL, menu_select,	31},
	{"/Help/www.gphoto.org/Ne_ws",			                NULL, menu_select,	33},
	{"/Help/www.gphoto.org/Updates_",			        NULL, menu_select,	34},
	{"/Help/www.gphoto.org/Cameras_",			        NULL, menu_select,	35},
	{"/Help/www.gphoto.org/Supporting_",			        NULL, menu_select,	36},
	{"/Help/www.gphoto.org/Di_scussion",			        NULL, menu_select,	37},
	{"/Help/www.gphoto.org/Devel. Team_",			        NULL, menu_select,	38},
	{"/Help/www.gphoto.org/Themes_",			        NULL, menu_select,	39},
	{"/Help/www.gphoto.org/Lin_ks",			        NULL, menu_select,	40},
	{"/Help/www.gphoto.org/To Do_",			        NULL, menu_select,	41},
	{"/Help/www.gphoto.org/Feed_back",			        NULL, menu_select,	42},

};


void create_menu (GtkWidget *menu_bar) {

	GtkItemFactory *item_factory;
	int menu_items_size = sizeof(menu_items) / sizeof (menu_items[0]);

	mainag=gtk_accel_group_new();

	item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<gp>", mainag);
	gtk_item_factory_create_items(item_factory, menu_items_size, menu_items, NULL);
	gtk_box_pack_start (GTK_BOX(menu_bar), gtk_item_factory_get_widget(item_factory, "<gp>"),
			FALSE, FALSE, 0);
}
