/*


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

#include <sys/ioctl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "serio.h"

#if !defined(STAND_ALONE)
# include "../../src/gphoto.h"
# include "../../src/util.h"
#else
char *serial_port;
char serialbuff[64];
#endif


static const char version_string[] = "0.1.0";



/**************************************************************
*
*/
int decode_image(char *filename, int image_style );
int Converse(Packet *out, unsigned char *str, int len, int chg_ctrl, int );



static int MAXTIME = 2;

static unsigned int START_PACKET	= 192;
static unsigned int END_PACKET		= 193;
static unsigned int IMAGE_JPG		= 0x0C;
static unsigned int THUMB_JPG		= 0x248;

static unsigned char ESCAPE_CHAR	= 0x7d;


static int PORT_SPEED	= B9600;


static int	NO_CTRL		= 0;
static int	CHG_CTRL	= 1;
static int	IN_PACKET	= 0;


static unsigned char	dsc_controlchar;

Packet	CameraInvalid = {0, 2, {131,125}, 93};
//Packet	CheckSumInvalid = {0, 2, {137,193}, 93};
 

#if defined(USE_ALL_TYPES)
static unsigned char	EmailImage[] = {0,2,2,0,16,'/','M','S','S','O','N','Y','/','I','M','C','I','F','1','0','0'};
static unsigned char	MpegImage[] = {0,2,2,0,16,'/','M','S','S','O','N','Y','/','M','O','M','L','0','0','0','1'};
#endif


static unsigned char	IdentString[] = {0,1,1,'S','O','N','Y',' ',' ',' ',' ',' '};
static unsigned char	EmptyPacket[]		= {0}; /* null packet */
static unsigned char	SetTransferRate[]	= {0,1,3,0};
static unsigned char	SendImage[]		= {0,2,'1',0,1,0,0};
static unsigned char	SendNextImagePacket[]	= {0,2,'1',0};
static unsigned char	SendThumbnail[]		= {0,2,'0',0};
static unsigned char	SelectImage[]		= {0,2,48,0,0,0,0};

//static unsigned char	SelectCamera[]		= {0,1,2};
//static unsigned char	DownloadComplete[]	= {0,2,'0',255};
//static unsigned char	X10Camera[]		= {0,1,5};

static unsigned char	StillImage[] = {0,2,2,0,14,'/','D','C','I','M','/','1','0','0','M','S','D','C','F'};


static unsigned char	X5Camera[]		= {0,2,1};
static unsigned char	X13Camera[]		= {0,2,18};
//static unsigned char	SendThumbnail[]		= {0,2,'0',0};





/*******************************************************************
*
*
*
*/
int sony_dscf55_initialize()
{
	printf("Init\n");

	PORT_SPEED = ConfigDSCF55Speed();

	if(InitSonyDSCF55(serial_port))
	{
		int count = 0;
		Packet dp;

		for(count=0; count<3; count++)
		{
			Converse(&dp, EmptyPacket, 1, CHG_CTRL,1);
			if(Converse(&dp, IdentString, 12, NO_CTRL,1))
				break;
	
			usleep(2000);
//			Converse(&dp, EmptyPacket, 1, CHG_CTRL,1);
			printf("Init - Fail %u\n", count+1);
		}

		if(count<5)
		{
			return TRUE;
		}

	}

	printf("Init - leaving\n");

	return FALSE;
}


/*******************************************************************
*
*
*
*/
struct Image *sony_dscf55_get_preview()
{
	return FALSE;
}


/*******************************************************************
*
* This may be possible but I have no information on how to do this.
*
*/
int sony_dscf55_delete_picture(int number)
{
	return FALSE;
}


/*******************************************************************
*
* This may be possible but I have no information on how to do this.
*
*/
int sony_dscf55_take_picture()
{
	return FALSE;
}


/*******************************************************************
*
*
*
*/
int sony_dscf55_number_of_pictures()
{
	Packet dp;

	Converse(&dp, SetTransferRate, 4, CHG_CTRL, 1);

	if(Converse(&dp, StillImage, 19, CHG_CTRL, 1))
	{

		if(Converse(&dp, X5Camera, 3, CHG_CTRL, 1))
		{
			return (int) dp.buffer[5];
		}
		else
			fprintf(stderr, "X5Camera Failed\n");

	}
	else
		fprintf(stderr, "StillImage Failed\n");

	return 0;

}


/*******************************************************************
*
*
*
*/
int sony_dscf55_configure()
{
	return 1;
}


/*******************************************************************
*
*
*
*/
char *sony_dscf55_summary()
{
	return "";
}


/*******************************************************************
*
*
*
*/
char *sony_dscf55_description()
{
	return "Sony DSC F55\nStill image support";
}


/*******************************************************************
*
*
*
*/
int WritePacket(FILE *fp, Packet *p)
{
	fwrite(&START_PACKET, sizeof(char), 1, fp);
	fwrite((char *)p->buffer, sizeof(char), p->length, fp);
	fwrite((char *)&p->checksum, sizeof(char), 1, fp);
	fwrite(&END_PACKET, sizeof(char), 1, fp);
	return 0;
}


/*******************************************************************
*
*
*
*/
int ReadFilePacket(FILE *fp, Packet *p)
{
	unsigned char c;
	unsigned char last_char;

	while(!feof(fp))
	{
		fread(&c, 1, 1, fp);

		if(START_PACKET == c)
		{
			p->length = 0;
			break;
		}
	}

	fread(&last_char, 1, 1, fp);

	while(!feof(fp))
	{
		fread(&c, 1, 1, fp);

		if(END_PACKET == c)
		{
			p->checksum = last_char;
			break;
		}

		p->buffer[p->length++] = last_char;

		last_char = c;
	}

	return 0;
}


/*******************************************************************
*
*
*
*/
int ReadCommsPacket(Packet *dst)
{
	unsigned short int count = 0;
	int length = 128;
	unsigned char buffer[256];

	dst->length = 0;

//	usleep(1000);

	do
	{
		length = Read(buffer, &length );

		if(length)
		{	

			if(buffer[0] == START_PACKET)
			{
				memcpy(dst->buffer, buffer+1, length);
				dst->length += length-1;

				dsc_controlchar = buffer[1];
				
				count = 0;

				if(*(buffer+length-1) != END_PACKET)
					continue;

				dst->length--;
				dst->checksum = dst->buffer[--dst->length];
				return 1;
			}
			else
			{
				if(dst->length)
				{
					memcpy(dst->buffer+dst->length, buffer, length);
					dst->length += length;

					count = 0;

					if(*(buffer+length-1) != END_PACKET)
						continue;

					dst->length-=2;
					dst->checksum = dst->buffer[dst->length];
					return 1;
				}
			}
		}
		else
		{
			fprintf(stderr, "Read failed in ReadCommsPacket\n");
		}

		count++;
	}while(count<MAXTIME);

	return 0;
}


/*******************************************************************
*
*
*
*/
unsigned char CalcCheckSum(Packet *p)
{
	unsigned short int o = 0;
	unsigned char sum = 0;

	while(o != p->length)
		sum += p->buffer[o++];

	return 256-(sum&255);
}


/*******************************************************************
*
*
*
*/
int AddCheckSum(Packet *p)
{

	do
	{
		unsigned char c = CalcCheckSum(p);

		if(c == END_PACKET)
		{
			p->buffer[0] = 14;
			continue;
		}

		p->checksum = c;
		break;
	}while(1);

	return TRUE;
}


/*******************************************************************
*
*
*
*/
int MakePacket(Packet *p, unsigned char *buffer, unsigned short int length)
{
	p->length = 0;

	while(length--)	
		p->buffer[p->length++] = *(buffer++);

	p->checksum = CalcCheckSum(p);

	return TRUE;
}


/*******************************************************************
*
*
*
*/
int CheckPacket(Packet *p)
{
	unsigned char c = CalcCheckSum(p);

	if(c==p->checksum)
	{
		return TRUE;
	}

	return FALSE;
}


/*******************************************************************
*
*
*
*/
int ComparePacket(Packet *s, Packet *d)
{
	if(s->length == d->length)
		if(s->checksum == d->checksum)
		{
			if(!memcmp(s->buffer, d->buffer, d->length))
				return TRUE;
			else
				printf("memcmp failed\n");
		}
		else
			printf("checksum failed\n");
	else
		printf("length failed\n");

	return FALSE;
}

/*******************************************************************
*
*
*
*/
void SendPacket(Packet *p)
{
	Write((char *)&START_PACKET, 1);
	Write((char *)p->buffer, p->length);
	Write((char *)&p->checksum, 1);
	Write((char *)&END_PACKET, 1);
}



/*******************************************************************
*
*
*
*/
int Converse(Packet *out, unsigned char *str, int len, int chg_ctrl, int check_packet)
{
	Packet ps;
	int count;

	MakePacket(&ps, str, len);

	if(CHG_CTRL == chg_ctrl)
	{
		if(dsc_controlchar !=14)
			ps.buffer[0] = dsc_controlchar+2;
		else
			ps.buffer[0] = 0;

		AddCheckSum(&ps);
	}


	for(count=0; count<3; count++)
	{
		SendPacket(&ps);

		if(ReadCommsPacket(out))
			if(check_packet)
			{
				if(CheckPacket(out))
					return 1;
			}
			else
				return TRUE;
	}

	return FALSE;
}


/*******************************************************************
*
*
*
*/
int decode_image(char *filename, int image_style)
{
	FILE *fp;
	FILE *fp1;
	unsigned char	value;
	int count = 0;

	rename(filename, "/tmp/tempfile");

	fp = fopen("/tmp/tempfile", "rb");
	fp1 = fopen(filename, "wb");

	if(!(fp && fp1))
	{
		if(fp)
		{
			fclose(fp);
			fprintf(stderr, "Could not open image source\n");
		}

		if(fp1)
		{
			fclose(fp1);
			fprintf(stderr, "Could not open image destination\n");
		}

		return FALSE;
	}

	IN_PACKET = 0;

	while(fread(&value, sizeof(char), 1, fp))
	{
		if((IN_PACKET == 0) && (value == START_PACKET))
		{
			char buffer[8];

			IN_PACKET = 1;

			if(fread(buffer, sizeof(char), 7, fp) != 7)
			{
			}

			continue;
		}

		if(!IN_PACKET)
			continue;

		if(value == END_PACKET)
		{
			/* lose checksum */
			fseek(fp1, -1L, SEEK_CUR);
			IN_PACKET = 0;
			continue;
		}

		if(0 == count)
		{
			fseek(fp, image_style, SEEK_SET);
			count++;
			continue;
		}

		if(ESCAPE_CHAR == value)
		{
			unsigned char extra;

			fread(&extra, sizeof(char), 1, fp);

			switch(extra)
			{
				case 1:
				case 7:
				case 0xe1:
				case 0xe0:
					extra &= 0xcf;
					fwrite(&extra, sizeof(char), 1, fp1);
					break;
				case 0x5d:
					fwrite(&value, sizeof(char), 1, fp1);
					break;
				default:
					fwrite(&value, sizeof(char), 1, fp1);
					fwrite(&extra, sizeof(char), 1, fp1);
					break;
			}
	
			continue;
		}

		fwrite(&value, sizeof(char), 1, fp1);
	}

	fclose(fp);
	fclose(fp1);

	return (IN_PACKET==0) ? TRUE : FALSE;
}



#if !defined(STAND_ALONE)


/*******************************************************************
*
*
*
*/
struct Image *sony_dscf55_get_picture(int imageid, int thumbnail)
{
	static FILE *fp;
	char filename[64];
	long size;
	struct Image *image;
	FILE *temp;
	Packet dp;




	if(PORT_SPEED == B115200)
	{
		SetTransferRate[3] = 4;
		Converse(&dp, SetTransferRate, 4, CHG_CTRL, 1);
		dscSetSpeed(B115200);
		Converse(&dp, EmptyPacket, 1, CHG_CTRL, 1);
	}


	if(thumbnail)
	{

		if(!Converse(&dp, StillImage, 19, CHG_CTRL, 1))
			printf("StillImage Failed\n");

		if(!Converse(&dp, X5Camera, 3, CHG_CTRL, 1))
			printf("X5Camera Failed\n");
		
		if(!Converse(&dp, X13Camera, 3, CHG_CTRL, 1))
			printf("X13Camera Failed\n");

		SelectImage[4] = imageid;
		Converse(&dp, SelectImage, 7, CHG_CTRL, 1);

		sprintf(filename, "/tmp/gphoto_image_%u.jpg", imageid-1);
		printf("%s\n", filename);
//		sprintf(filename, "/tmp/%s", dp.buffer+5);
		temp = fopen(filename, "wb");

		Converse(&dp, SendThumbnail, 4, CHG_CTRL, 0);
		WritePacket(temp, &dp);
		memset(dp.buffer, 0, sizeof(dp.buffer));
		Converse(&dp, SendThumbnail, 4, CHG_CTRL, 0);
		WritePacket(temp, &dp);

		fclose(temp);

		decode_image(filename, THUMB_JPG);
	}
	else
	{
		int last_packet = 0;

		sprintf(filename, "/tmp/gphoto_image_%u.jpg", imageid-1);

		if(!Converse(&dp, StillImage, 19, CHG_CTRL, 1))
			printf("StillImage Failed\n");

		SendImage[4] = imageid;

		Converse(&dp, SendImage, 7, CHG_CTRL, 0);

		temp = fopen(filename, "wb");

		do
		{
			WritePacket(temp, &dp);
			memset(dp.buffer, 0, sizeof(dp.buffer));
			Converse(&dp, SendNextImagePacket, 4, CHG_CTRL, 0);

			if(last_packet == 1)
				break;

			if(dp.buffer[4] == 3)
				last_packet = 1;

		}
		while(1);

		fclose(temp);
		decode_image(filename, IMAGE_JPG);
	}



	fp = fopen(filename, "r");

	image = 0;

	if(fp)
	{
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		rewind(fp);
		image = (struct Image*)malloc(sizeof(struct Image));

		image->image = (char*)malloc(sizeof(char)*size);
		image->image_size = size;
		image->image_info_size = 0;
		fread(image->image, sizeof(char), size, fp);
		unlink(filename);
	}
	else
		printf("Failed to open file\n");

	SetTransferRate[3] = 0;
	Converse(&dp, SetTransferRate, 4, CHG_CTRL, 1);
	dscSetSpeed(B9600);
	Converse(&dp, EmptyPacket, 1, CHG_CTRL, 1);

	return(image);
}



struct _Camera sony_dscf55 = {  sony_dscf55_initialize,
			       sony_dscf55_get_picture,
			       sony_dscf55_get_preview,
			       sony_dscf55_delete_picture,
			       sony_dscf55_take_picture,
			       sony_dscf55_number_of_pictures,
			       sony_dscf55_configure,
			       sony_dscf55_summary,
			       sony_dscf55_description};



#else

/*******************************************************************
*
*
*
*/
int sony_dscf55_store_picture(int imageid, char *path)
{
	FILE *fp;
	FILE *temp;
	int last_packet = 0;
	int f;
	char filename[64];
	long size;
	struct Image *image;
	Packet dp;


	if(PORT_SPEED == B115200)
	{
		SetTransferRate[3] = 4;
		Converse(&dp, SetTransferRate, 4, CHG_CTRL, 1);
		dscSetSpeed(B115200);
		Converse(&dp, EmptyPacket, 1, CHG_CTRL, 1);
	}


	SelectImage[4] = imageid;
	Converse(&dp, SelectImage, 7, CHG_CTRL, 1);

	sprintf(filename, "%s/%s", path, dp.buffer+5);

	f = strlen(filename);
	filename[f] = filename[f-1];
	filename[f-1] = filename[f-2];
	filename[f-2] = filename[f-3];
	filename[f-3] = '.';
	filename[f+1] = 0;

	f = 0;

	while(filename[f])
	{
		filename[f] = tolower(filename[f]);
		f++;
	}

	temp = fopen(filename, "wb");

	if(!Converse(&dp, StillImage, 19, CHG_CTRL, 1))
		printf("StillImage Failed\n");

	SendImage[4] = imageid;
	Converse(&dp, SendImage, 7, CHG_CTRL, 0);

	temp = fopen(filename, "wb");
	printf("%s\n", filename);

	do
	{
		WritePacket(temp, &dp);
		memset(dp.buffer, 0, sizeof(dp.buffer));
		Converse(&dp, SendNextImagePacket, 4, CHG_CTRL, 0);

		if(last_packet == 1)
			break;

		if(dp.buffer[4] == 3)
			last_packet = 1;

	}
	while(1);

	fclose(temp);

	SetTransferRate[3] = 0;
	Converse(&dp, SetTransferRate, 4, CHG_CTRL, 1);
	dscSetSpeed(B9600);
	Converse(&dp, EmptyPacket, 1, CHG_CTRL, 1);

	return decode_image(filename, IMAGE_JPG);
//	return FALSE;
}


/*******************************************************************
*
*
*
*/
int main(int argc, char *argv[])
{
	char path[64];
	int totalpics;
	int count;
	serial_port = serialbuff;

	if(2 != argc)
	{
		fprintf(stderr, "Usage : %s <dir>\n", argv[0]);
		exit(1);
	}

	strncpy(path, argv[1], 64);

	if(!ConfigDSCF55Port(serial_port, 64))
	{
		fprintf(stderr, "Unable to set serial port\n");
		exit(2);
	}

	if(!sony_dscf55_initialize())
	{
		fprintf(stderr, "Unable to initialize camera\n");
		exit(2);
	}


	if(!(totalpics=sony_dscf55_number_of_pictures()))
	{
		fprintf(stderr, "Camera error or no pictures to download\n");
		exit(2);
	}


	for(count=1; count<=totalpics; count++)
	{

		int s = sony_dscf55_store_picture(count, path);

		if(!s)
		{
			fprintf(stderr, "Error downloading image\n");
			exit(3);
		}
	}


	return count;

}

#endif

