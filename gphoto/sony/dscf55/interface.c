/*

gPhoto Programmer's Manual (v0.5)  		    Rev. 2 (1999-10-06)
-----------------------------------------------------------------------

What follows is a description of the functions that gPhoto uses via the
common camera library interface. If you are interested in writing a new
front-end for the gPhoto libraries, then this document is for you. 

The front-end provided functions are functions that need to be available
to the camera libraries. The libraries will call these functions to 
send status reports, or get/send camera configuration options.

The Camera library interface is the defined set of functions that a
camera library must implement and make available to the GUI. It is
possible that all the functions may not be supported by the camera, in
which case the library can simply return the unsuccessful value.

Functions Provided by the GUI
-----------------------------------------------------------------------

	Displaying Information
	----------------------

	The following functions are used by the camera libraries to update
	the GUI and/or user on the status of the current operation.

		gp_update_status (char *status);
			Sets the text in a status bar. Usually this 
			is "working" information, such as
			"Scanning for camera..."

		gp_update_progress (float percentage);
			Used to update a progress bar, with a percentage
			between 0 and 1.
		gp_message (char *message);
			Displays the message.

	Camera Library Configuration Key/Value
	--------------------------------------
	The following functions allow libraries to save configuration
	options through the GUI. The config values here should be internal
	to the library, meaning they can not be retrieved from the camera.
	Items such as "Debug Mode", or "Baud Rate" would be ideal for
	this.

	This allows the GUI full control on where to place the
	configuration data. It could save the info to a rc file, or for
	the example of a GNOME GUI, it could pass the information right
	along to GNOME's internal config functions.

		char *gp_read_setting (char *key);
			returns the library setting option 
			specified by the "key". Example: to get the
			value for the setting key "casio_baud",
			call gp_read_setting("casio_baud");

		void gp_save_setting (char *key, char *value);
			saves the configuration option specified by 
			"key" as "value". Example: to save the
			"casio_baud" value as "57600", call
			gp_save_setting("casio_baud", "57600");

Camera Library Interface
-----------------------------------------------------------------------
	Notice that all functions return GPHOTO_ERROR if the calls was
	unsuccessful.
	
	CameraAbilities* get_abilities ();
		Returns the camera's abilities. This will tell the
		front-end exactly what the library can and can't do.

		return values:
			unsuccessful : GPHOTO_ERROR
			  successful : valid CameraAbilities struct

	int open (CameraInit*); 
		Initializes the library.

		return values: 
			unsuccessful : GPHOTO_ERROR
			  successful : GPHOTO_OK

	int close ();
		Closes out the camera library. Called when the
		application closes, or when a different camera 
		library is selected.
			unsuccessful : GPHOTO_ERROR
			  successful : GPHOTO_OK
		
	int number_of_data ();
		Returns the number of "data" on the camera. This
		is the total number of pictures, and also audio
		and/or video if supported.

		return values:
			unsuccessful : GPHOTO_ERROR
			  successful : number of data

	CameraData* get_data (int data_number, int thumbnail);
		Returns data #data_number from the camera.
		Can return a picture, audio, or video. The thumbnail
		option can be ignored if there is no way to get a 
		thumbnail (e.g.: audio).

		If thumbnail == TRUE, gets the thumbnail.
		If thumbnail == FALSE, gets the full-size data.

		return values:
			unsuccessful : GPHOTO_ERROR
			  successful : valid CameraData struct

	CameraData* get_preview ();
		Returns a preview of what the camera currently "sees"
		or "hears" if possible.

		return values:
			unsuccessful : GPHOTO_ERROR
			  successful : valid CameraData struct

	int delete_data (int data_number);
		Deletes data #data_number from the camera.

		return values:
			unsuccessful : GPHOTO_ERROR
			  successful : GPHOTO_OK

	char *get_camera_config ();
		Returns a string with the meta-widgets.
		Each meta-widget must be on it's own line. Examples below
		in the "Meta-widgets" section.

		return values:
			unsuccessful : GPHOTO_ERROR
			  successful : valid meta-widget string

	int set_camera_config (char *config_settings);
		Sets the configuration for the camera library with
		config_settings being a string with "key=value" pairs
		each on a separate line (separated by \n).
		
		return values:
			unsuccessful : GPHOTO_ERROR
			  successful : GPHOTO_OK

	int take_data (CameraDataType);
		The camera actually takes a data (picture, audio, etc).

		return values:
			unsuccessful : GPHOTO_ERROR
			  successful : GPHOTO_OK

	char *summary (); 
		Returns summary information about the current state of
		the cameras. Examples would be free memory, battery 
		remaining, pictures on camera, pictures remaining, etc.

		return values: 
			unsuccessful : GPHOTO_ERROR
			  successful : Full camera summary

	char *manual ();
		Returns text on how to use the camera library, including
		unsupported functionality, or any issues involved with	
		the camera. The important item to put in here would be
		use of the configuration options. 

			unsuccessful : GPHOTO_ERROR
			  successful : Full library manual

	char *about (); 
		Returns a camera library description, that includes
		the author's name/email-address, and other relavent
		information.

		return values: 
			unsuccessful : GPHOTO_ERROR
			  successful : Full library description

Program Flow
-----------------------------------------------------------------------

Here is the program flow:

	1) User selects a library to use
	2) "get_abilities" is called to determine what the camera can and
	   can't do.
	3) "open" is called to set the baud rate (based on findings from
	   the abilities) and the serial-port.
	4) User performs operations (get_data, delete_data, etc.)
	5) "close" is called when another library is selected, or
	   when the program is closed down. This is a good time to
	   save settings using gp_save_setting, or to power-down the
	   camera if possible. basically, clean-up.


Meta-widgets
-----------------------------------------------------------------------
In order to be as toolkit independent as possible, the library's
"get_camera_config" returns what are called "meta-widgets". All
these are are a description of the options that the user should be
presented with, and to choose from. They include the basic 4 input 
widgets:
	toggle button, radio button, text entry, and a range selection.

as well as 4 layout widgets:
	page, frame, line, label

The string that get_configuration returns contains the above tags. This
can be thought of as a "form" in HTML. What follows is a description and 
usage for each meta-widget:

	Layout Widgets
	--------------
	The layout is a "flow" layout, meaning if you send a layout
	widget, all input widgets will be placed in that layout widget
	until you tell it to stop, one right under another. You don't NEED
	any of the following because the GUI should simply place the Input
	Widgets in the dialog if no Layout Widgets are specified.

		page("name");
			Create a page with the title "name".
			To stop placing widgets in this page,
			send "page(END);"

		frame("name");
			Create a frame with the title "name".
			To stop placing widgets in this frame,
			send "frame(END);"

		label("name");
			Create a label that displays the text "name".

		line();
			Create a horizontal separator line.

	Input Widgets
	-------------
	The Input Widgets are simply ways for the user to choose
	or enter options.

		toggle("name", value);
			Create a toggle button named "name". 
			If value == 0, the toggle button is up.
			If value == 1, the toggle button is down.

		radio("group", "name", value);
			Create a radio button, placed in group "group"
			with "name" being that particular radio button's
			name.
			If value == 0, the radio button is up.
			If value == 1, the radio button is down.
			(Note: remember only 1 radio button in a
			particular group can be down.)

		entry("name", "value", length);
			Create a text entry named "name", with the
			the preset text being "value". The length
			specifies the maximum number of allowed
			characters.

		range("name", value, low, high);
			Create a range (slider) that lets the user
			select a number between low and high. It
			is preset to value.

Example:
	Here is an example string that could be returned from
	get_configuration. It creates a page named "Basic", 
	that contains a radio group name "Baud" to select the baud rate, 
	inserts a line after that, and then and a text entry to enter the
	camera name. It then creates a new page named "Advanced" that has
	a frame named "Power Settings" which contains a range to select
	the camera timeout when the camera is connected to the computer
	with values from 0 to 600 (preset at 200), and a range to select
	the camera timeout when it is not connected with values from 0 to
	300 (preset at 150). The call to get_configuration() would return
	this as a text string:

page("Basic");
radio("Baud", "9600",  1);
radio("Baud", "14400", 0);
radio("Baud", "28800", 0);
radio("Baud", "57600", 0);
line();
entry("Camera Name", "Scott's Camera", 32);
page(END);
page("Advanced");
frame("Power Settings");
range("Connected", 200, 0, 600);
range("Disconnected", 150, 0, 300);
frame(END);
page(END);

	Remember that each meta-widget must be on its own line.	

	And here is the same thing, except I indented it to be a little more
	readable. This should show you how the layout widgets break
	things up:

page("Basic");
	radio("Baud", "9600",  1);
	radio("Baud", "14400", 0);
	radio("Baud", "28800", 0);
	radio("Baud", "57600", 0);
	line();
	entry("Camera Name", "Scott's Camera", 32);
page(END);

page("Advanced");
	frame("Power Settings");
		range("Connected", 200, 0, 600);
		range("Disconnected", 150, 0, 300);
	frame(END);
page(END);


Library Quick Reference
------------------------------------------------------------------
Here is a quick listing of the library functions:

	CameraAbilities* get_abilities();
	int 		 open (CameraInit*); 
        int 		 close ();
        int 		 number_of_data ();
        CameraData* 	 get_data (int data_number, int thumbnail);
        CameraData* 	 get_preview ();
        int 		 delete_data (int data_number);
        char*		 get_camera_config ();
        int		 set_camera_config (char *config_settings);
        int 		 take_data (CameraDataType);
        char*		 summary (); 
        char*		 manual ();
        char*		 about (); 

And the functions available to the libraries:

	void 		 gp_update_status (char *status);
	void 		 gp_update_progress (float percentage);
	void		 gp_message (char *message);
	char*		 gp_read_setting (char *key);
	void 		 gp_save_setting (char *key, char *value);



------------------------------------------------------------------
Copyright (C) 1998-99 Scott Fritzinger (scottf@scs.unr.edu)
Copyright (C) 1998-99 Ole Kristian Aamot (oleaa@ifi.uio.no)
Copyright (C) 1999    Phill Hugo (phill@gphoto.org)

Verbatim copying and distribution of this entire text is permitted 
in any medium, provided this notice is preserved.
*/


typedef struct _tagCameraAbilities
{
	int port_speed;
} CameraAbilities;


typedef struct _tagCameraInit
{
	int port_speed;
} CameraInit;

typedef struct _tagCameraData
{
	int port_speed;
} CameraData;


#define GPHOTO_ERROR	0x1000





/*
*
*
*
*/
CameraAbilities* get_abilities()
{
	static ability;

	return (CameraAbilities *)0;
}



/*
*
*
*
*/
int open(CameraInit*p) 
{
	return GPHOTO_ERROR;
}


/*
*
*
*
*/
int close()
{
	return GPHOTO_ERROR;
}


/*
*
*
*
*/
int number_of_data()
{
	return GPHOTO_ERROR;
}


/*
*
*
*
*/
CameraData* get_data(int data_number, int thumbnail)
{
}


/*
*
*
*
*/
CameraData* get_preview()
{

	return (CameraData *)0;
}


/*
*
*
*
*/
int delete_data(int data_number)
{
	return GPHOTO_ERROR;
}


/*
*
*
*
*/
char* get_camera_config()
{
}


/*
*
*
*
*/
int set_camera_config(char *config_settings)
{
	return GPHOTO_ERROR;
}


/*
*
*
*
*/
int take_data(CameraDataType)
{


	return GPHOTO_ERROR;
}


/*
*
*
*
*/
char* summary() 
{
}


