#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>

#define CAMERA_EPOC              852094800

/* Control bytes */
#define PKT_CTRL_RECV            0x01
#define PKT_CTRL_SEND            0x00
#define PKT_CTRL_EOF             0x80
#define PKT_CTRL_CANCEL          0xFF


/* Kodak System Codes */
#define DC_COMMAND_COMPLETE      0x00
#define DC_COMMAND_ACK           0xD1
#define DC_CORRECT_PACKET        0xD2
#define DC_COMMAND_NAK           0xE1
#define DC_ILLEGAL_PACKET        0xE3
#define DC_BUSY                  0xF0
  
/* Kodak System Commands */
#define DC_SET_RESOLUTION        0x36
#define DC_SET_SPEED             0x41
#define DC_PICTURE_DOWNLOAD      0x64
#define DC_PICTURE_INFO          0x65
#define DC_PICTURE_THUMBNAIL     0x66
#define DC_SET_SOMETHING         0x75
#define DC_TAKE_PICTURE          0x7C
#define DC_ERASE                 0x7A
#define DC_ERASE_IMAGE_IN_CARD   0x7B
#define DC_INITIALIZE            0x7E
#define DC_STATUS                0x7F
#define DC_SET_CAMERA_ID         0x9E

struct kodak_dc210_status {
   char reserved_1;           	/* 1 */
   char camera_type_id;      	/* 2 */
   char firmware_major;      	/* 3 */
   char firmware_minor;      	/* 4 */
   char reserved_2[4];        	/* 5-8 */
   char batteryStatusId;     	/* 9 battery status */
   char acStatusId;          	/* 10 */
   char reserved_3[2];        	/* 11-12 */
   int  time;		     	/* 13-16 */
   char zoomMode;     	     	/* 17 */
   char reserved_4;           	/* 18 */
   char flash_charged;		/* 19 */
   char compression_mode_id;	/* 20 */
   char flash_mode;		/* 21 */
   char exposure_compensation;	/* 22 */
   char picture_size;		/* 23 */
   char file_Type;		/* 24 */
   char reserved_5;            	/* 25 */
   int totalPicturesTaken;     	/* 26-29 */
   int totalFlashesFired;      	/* 31-34 */
   char reserved_6[21];        	/* 35-55 */
   char num_pictures;         	/* 56-57 */
   char reserved_7[32];        	/* 58-89 */ /* should be 32 */
   char camera_ident[30];      	/* 90-120 */
   char reserved_8[135];       	/* 121-256 */
};

struct kodak_dc210_picture_info {
   char reserved_1[3];
   char resolution;
   char compression;
   char reserved_2;
   short  pictureNumber;
   int  fileSize;
   int  elapsedTime;
   char reserved_3[16];
   char fileName[12];
   char reserved_4[222];
};

