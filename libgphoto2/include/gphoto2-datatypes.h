/* 	Header file for gPhoto 0.5-Dev

	Author: Scott Fritzinger <scottf@unr.edu>

	This library is covered by the LGPL.
*/

/* Data Structures
   ---------------------------------------------------------------- */

/* Return Values */
#define	GP_OK				 0
#define GP_ERROR			-1
#define GP_ERROR_CRITICAL		-2

/* File Types */
typedef enum {
	GP_FILE_UNKNOWN,
	GP_FILE_JPEG,
	GP_FILE_TIFF,
	GP_FILE_FLASHPIX,
	GP_FILE_PPM,
	GP_FILE_WAV,
	GP_FILE_MPEG,
	GP_FILE_QUICKTIME
} CameraFileType;

/* Physical Connection Types */
typedef enum {
	GP_PORT_NONE,
	GP_PORT_SERIAL,
	GP_PORT_PARALLEL,
	GP_PORT_USB,
	GP_PORT_IEEE1394,
	GP_PORT_IRDA,
	GP_PORT_SOCKET
} CameraPortType;


typedef gpio_device_info CameraPortInfo;

typedef struct {
	char port[128];
		/* path to serial port device 			 */
		/* For serial port, "/dev/ttyS0" or variants	 */
		/* For parallel port, "/dev/lpt0" or variants	 */
		/* For usb, "inep# outep#"			 */
		/* For ieee1394, "ieee1394"			 */
		/* For socket, the address (ip or fqdn).	 */
		/* For directory, the path.			 */

	int speed;
		/* Speed to use					 */
} CameraPortSettings;

typedef struct {
	char model[128];

		/* can the library support the following: */
	int serial;
	int parallel;
	int usb;
	int ieee1394;
		/* set to 1 if supported, 0 if not.		 */

	int speed[64];
		/* if serial==1, baud rates that this camera	 */
		/* supports. terminate list with a zero 	 */

	int capture;
		/* Camera can do a capture (take picture) 	 */

	int config;
		/* Camera can be configures remotely 		 */

	int file_delete;
		/* Camera can delete files 			 */

	int file_preview;
		/* Camera can get file previews (thumbnails) 	 */

	int file_put;
		/* Camera can receive files			 */

} CameraAbilities;

typedef struct {
	char model[128]; 		   /* Name of the camera */

	CameraPortSettings port_settings; 	/* Port settings */
} CameraInit;

typedef struct {
	CameraFileType	type;
		/* Type of file (GP_FILE_JPEG, GP_FILE_TIFF, ..) */

	char		name[64];
		/* Suggested name for the file */

	long int	size;
		/* Size of the image data*/

	char*		data;
		/* Image data */

} CameraFile;

typedef struct {
	char	name[128];
} CameraFolderInfo;

typedef struct {
	int count;
	CameraFolderInfo entry[512];
} CameraFolderList;

typedef struct {
	char name[32];
	char value[32];
} CameraSetting;
