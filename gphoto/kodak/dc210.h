/* True/False */
#define TRUE			1
#define FALSE			0

#define SPD_VHI			1
#define SPD_LOW			0

/* SDK error codes */

#define RET_OK			1
#define RET_ERROR		0
#define RET_TIMEOUT		-1
#define EPOC			852094800

/* Kodak DC210 System Codes */

#define DC_COMMAND_COMPLETE	0x00
#define DC_COMMAND_ACK		0xD1
#define DC_CORRECT_PACKET	0xD2
#define DC_COMMAND_NAK		0xE1
#define DC_EXECUTION_ERROR	0xE2
#define DC_ILLEGAL_PACKET	0xE3
#define DC_CANCEL		0xE4
#define DC_BUSY			0xF0

/* Kodak Host Interface Command */
/* cmd:00:arg1:arg2:arg3:arg4:00:1A */
/* 8 bytes */

/* Kodak Camera -> Host Packet sizes */
#define PACKET_IMAGE_DATA	1024
#define PACKET_CARD_DATA	512
#define PACKET_STATUS_DATA	256
#define PACKET_ERROR_DATA	18
/* Kodak Host -> Camera Packet sizes */
#define PACKET_FIRMWARE_DATA	257
#define PACKET_CMD_DATA		60

struct kodak_dc210_status {
	char camera_type;		// 4 = DC210
	char firmware_major;
	char firmware_minor;
	char dsp_major;
	char dsp_minor;
	char mu_major;
	char mu_minor;
	char battery;			// 0:Full 1:Low 2:Empty
	char acstatus;			// 0:Absent 1:Present
	int  camera_time;
    	char zoom_position;
	char flash_status;
	char compression;		// 1:Low 2:Med 3:High
	char flash_mode;		// 5 modes
	char exp_compensation;
	char resolution;		// 0:640 1:1152
	char filetype;			// 3:JPEG 4:FlashPix
	int  total_pictures;
	int  total_flash;
	char timer_mode;		// 0:Off 1:On
	char memory_card_status;	// see below;
	char video_format;		// 0:NTSC 1:PAL
	char comm_mode;			// 0:Serial 1:IRDA
	int  pictures_in_camera;
	int  remaining_low;
	int  remaining_med;
	int  remaining_high;
	char volume_id[11];
	char camera_id[32];
};

/*
status.camera_type		= packet[1];
status.firmware_major		= packet[2];
status.firmware_minor		= packet[3];
status.dsp_major		= packet[4];
status.dsp_minor		= packet[5];
status.mu_major			= packet[6];
status.mu_minor			= packet[7];
status.battery			= packet[8];
status.acstatus			= packet[9];
status.camera_time = EPOC + (packet[12] * 0x1000000 + packet[13] * 0x10000 + packet[14] * 0x100 + packet[15]) / 2;
status.zoom_position		= packet[16];
status.flash_status		= packet[18];
status.compression		= packet[19];
status.flash_mode		= packet[20];
status.exp_compensation		= packet[21];
status.resolution		= packet[22];
status.filetype			= packet[23];
status.total_pictures		= packet[25] * 0x100 + packet[26];
status.total_flash		= packet[27] * 0x100 + packet[28];
status.timer_mode		= packet[29];
status.memory_card_status	= packet[30];
status.video_format		= packet[31];
status.comm_mode		= packet[32];
status.pictures_in_camera	= packet[56] * 0x100 + packet[57];
status.remaining_low		= packet[68] * 0x100 + packet[69];
status.remaining_mid		= packet[70] * 0x100 + packet[71];
status.remaining_high		= packet[72] * 0x100 + packet[73];
strncpy(status.volume_id,packet+77,11);
strncpy(status.camera_id,packet+90,32);
*/

struct kodak_dc210_picture_info {
	char camera_type;		// 4:DC210
	char file_type;			// 3:JPEG 4:FlashPix
	char resolution;		// 0:640 1:1152
	char compression;		// 1:Low 2:Med 3:High
	int  picture_number;
	int  picture_size;
	int  picture_time;		// since 12:00 12/31/1996
	char flash_used;		// 0:No 1:Yes
	char flash_mode;		// 5 modes
	char zoom_position;
	char f_number;
	char battery;			// 0:OK 1:Low 2:Empty
	int  exposure_time;
	char image_name[12];
};

/*
picinfo.camera_type		= packet[1];
picinfo.file_type		= packet[2];
picinfo.resolution		= packet[3];
picinfo.compression		= packet[4];
picinfo.picture_number		= packet[6] * 0x100 + packet[7];
picinfo.picture_size		= packet[8]  * 0x1000000 + 
				  packet[9]  * 0x10000 +
				  packet[10] * 0x100 +
				  packet[11];
picinfo.picture_time		= packet[12] * 0x1000000 +
				  packet[13] * 0x10000 +
				  packet[14] * 0x100 +
				  packet[15];
picinfo.flash_used		= packet[16];
picinfo.flash_mode		= packet[17];
picinfo.zoom_position		= packet[21];
picinfo.f_number		= packet[26];
picinfo.battery			= packet[27];
picinfo.exposure_time		= packet[28]  * 0x1000000 +
				  packet[29]  * 0x10000 +
				  packet[30] * 0x100 +
				  packet[31];
strncpy(picinfo.image_name,packet+32,12);
*/

/* Kodak Camera Commands */
/* Firmware download mode */
#define FIRMWARE_READ_DATA	0x13
#define FIRMWARE_WRITE_DATA	0x1D
#define FIRMWARE_ERASE_FLASH	0x1E
#define FIRMWARE_EXECUTE_PRG	0x3D
#define FIRMWARE_DOWNLOAD_END	0xAE
#define FIRMWARE_MODE_SET	0x8D // Switch to ROM mode?
#define ADJUST_MODE_SET		0xAE

/* Normal mode */
#define DC_EEPROM_WRITE		0x21
#define DC_EEPROM_READ		0x22
#define DC_WRITE_DATA		0x31
#define DC_READ_DATA		0x32

#define DC_RESET		0x8A
#define DC_GET_STATUS		0x7F
#define DC_GET_BATTERY		0x7E

#define DC_TAKE_PICTURE		0x7C
#define DC_ERASE_IMAGE		0x7B
#define DC_GET_PICTURE		0x64
#define DC_GET_PICINFO		0x65
#define DC_GET_THUMB		0x66

#define DC_SET_RESOLUTION	0x36
#define DC_SET_FORMAT		0x37
#define DC_SET_SPEED		0x41
#define DC_SET_QUALITY		0x71
#define DC_SET_FLASH		0x72
#define DC_SET_FOCUS		0x73
#define DC_SET_DELAY		0x74
#define DC_SET_TIME		0x75
#define DC_SET_ZOOM		0x78
#define DC_SET_EXPOSURE		0x80
#define DC_SET_CAMERAID		0x9E

#define DC_ALBUM_FILENAME	0x4A

#define DC_CARD_GET_PIC		0x91
#define DC_CARD_GET_PICINFO	0x92
#define DC_CARD_READ_THUMB	0x93
#define DC_CARD_FORMAT		0x95
#define DC_CARD_OPEN		0x96
#define DC_CARD_CLOSE		0x97
#define DC_CARD_GET_STATUS	0x98
#define DC_CARD_GET_DIRECTORY	0x99
#define DC_CARD_FILE_READ	0x9A
#define DC_CARD_FILE_DEL	0x9D


int kodak_dc210_open_camera (const char *devname);
int kodak_dc210_read (unsigned char *buf, int nbytes );
int kodak_dc210_read_packet ( char *packet, int length);
unsigned char kodak_dc210_checksum(char *packet,int length);
int kodak_dc210_send_command ( char , int , int , int , int );
int kodak_dc210_command_complete(void);
void kodak_dc210_camera_init(void);


void eprintf(char *fmt, ...);

