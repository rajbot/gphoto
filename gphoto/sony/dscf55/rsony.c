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


int ReadCommByte(unsigned char *byte);
int ReadFileByte(unsigned char *byte);
Packet *ReadPacket(int(*readfunc(unsigned char *)));


static const char version_string[] = "0.3.0";

static int MAXTIME = 2;

static unsigned char START_PACKET	= 192;
static unsigned char END_PACKET		= 193;

char ESC_START_STRING[]	= { 0x7d, 0xe0 };
char ESC_END_STRING[]	= { 0x7d, 0xe1 };
char ESC_ESC_STRING[]	= { 0x7d, 0x5d };


static FILE	*dsc_global_file_fd;

/*------------------------------------------------------------------------
*
* This array contains the expected packet sequence code to to be applied/
* checked for.
*
*
*/
static const unsigned char dsc_sequence[] = {14,0,32,34,66,68,100,102,134,136,168,170,202,204,236,238,255};


/*------------------------------------------------------------------------
*
* This array is used by ReadPacket
*
*
*/
unsigned char PacketCodes[2] = {192,193};


/**************************************************************
*
*/
int decode_image(char *filename, int image_style );
int Converse(Packet *out, unsigned char *str, int len);



#define DSC_INVALID_CHECKSUM	0x40
#define DSC_INVALID_SEQUENCE	0x41
#define DSC_RESET_SEQUENCE	0x42
#define DSC_RESEND_PACKET	0x43

#define DSC_ESCAPE_CHAR		0x7d
#define DSC_START_CHAR		0xc0
#define DSC_END_CHAR		0xc1




static int PORT_SPEED	= B9600;

static int	NO_CTRL		= 0;
static int	CHG_CTRL	= 1;
static int	IN_PACKET	= 0;

static unsigned char		dscf55_controlchar;
static unsigned short int	dscf55_image_count;
static unsigned short int	dscf55_sequence_id;




Packet	CameraInvalid =	{0, 2, {131,125}, 93};
Packet	ResendPacket =	{0, 4, {129,2,'1',0}, 'L'};
 

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


/*static unsigned char	X5Camera[]		= {0,2,1};*/
static unsigned char	SendImageCount[]		= {0,2,1};
static unsigned char	X13Camera[]		= {0,2,18};



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
			dscf55_sequence_id = 0;

			if(Converse(&dp, IdentString,12))
				break;
	
			usleep(2000);
			printf("Init - Fail %u\n", count+1);
		}

		if(count<3)
			return TRUE;
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
* Return count of images taken.
*
*/
int sony_dscf55_number_of_pictures()
{
	Packet dp;

	Converse(&dp, SetTransferRate, 4);

	if(Converse(&dp, StillImage, 19))
	{

		if(Converse(&dp, SendImageCount, 3))
		{
			dscf55_image_count = (unsigned short int) dp.buffer[5];
			return (int) dp.buffer[5];
		}
		else
			fprintf(stderr, "SendImageCount Failed\n");

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
int ReadFileByte(unsigned char *byte)
{
	int status = fread(byte, 1, 1, dsc_global_file_fd);
}


/*******************************************************************
*
*
*
*/
Packet *ReadPacket(int(*readfunc(unsigned char *)))
{
	unsigned int n;
	unsigned int o;
	unsigned char byte=0;
	static Packet p;

	p.length = 0;

	for(n=0; n<2; n++)
	{
		for(byte=0;byte!=(unsigned char)PacketCodes[n];)
		{
			if(readfunc(&byte) <=0)
				return (Packet *)0;

			if(n>0)
				if(DSC_ESCAPE_CHAR == byte)
				{
					unsigned char extra;

					readfunc(&extra);

					switch(extra)
					{
						case 1:
						case 7:
						case 0xe1:
						case 0xe0:
							extra &= 0xcf;
							p.buffer[p.length++]=extra;
							continue;
						case 0x5d:
							p.buffer[p.length++]=byte;
							continue;
						default:
							p.buffer[p.length++]=byte;
							p.buffer[p.length++]=extra;
							continue;
					}
				}
				else
					p.buffer[p.length++]=byte;
		}
	}

	p.length-=2;
	p.checksum = p.buffer[p.length];

	return &p;
}


/*******************************************************************
*
*
*
*/
int ReadCommsPacket(Packet *dst)
{
	Packet *p;

	p = ReadPacket(ReadCommByte);

	if(p)
	{
		memcpy(dst, p, sizeof(Packet));
		return 1;
	}

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
	unsigned long int sum = 0;

	sum = 0;

	while(o < p->length)
		sum += p->buffer[o++];

	return 256-(sum&255);
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

	if(255==dsc_sequence[++dscf55_sequence_id])
		dscf55_sequence_id = 0;

	p->buffer[0] = dsc_sequence[dscf55_sequence_id++];

	if(255==dsc_sequence[dscf55_sequence_id])
		dscf55_sequence_id = 0;

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

	if(c!=p->checksum)
		return DSC_INVALID_CHECKSUM;

	if(129==p->buffer[0])
		return DSC_RESEND_PACKET;

	if(dsc_sequence[dscf55_sequence_id] != p->buffer[0])
		return DSC_INVALID_SEQUENCE;

	return TRUE;
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
	unsigned short int count;

	Write((char *)&START_PACKET, 1);

	p->buffer[p->length] = p->checksum;

	for(count=0; count<p->length+1; count++)
	{
		switch((unsigned char)p->buffer[count])
		{
			case DSC_ESCAPE_CHAR:
				Write(ESC_ESC_STRING, 2);
				break;

			case DSC_START_CHAR:
				Write(ESC_START_STRING, 2);
				break;

			case DSC_END_CHAR:
				Write(ESC_END_STRING, 2);
				break;

			default:
				Write((char *)&p->buffer[count], 1);
				break;
		}
	}

	Write((char *)&END_PACKET, 1);
}


/*******************************************************************
*
*
*
*/
int Converse(Packet *out, unsigned char *str, int len)
{
	Packet ps;
	char old_sequence=33;
	int sequence_count=0;
	int count;

	MakePacket(&ps, str, len);

	for(count=0; count<10; count++)
	{

		SendPacket(&ps);

		if(ReadCommsPacket(out))
			switch(CheckPacket(out))
			{
				case DSC_INVALID_CHECKSUM:
					printf("Checksum invalid\n");
					ps.buffer[0] = 129;
					ps.checksum = CalcCheckSum(&ps);
					break;

				case DSC_INVALID_SEQUENCE:
					if(old_sequence == out->buffer[0])
						sequence_count++;
					else
						if(0==sequence_count)
							old_sequence = out->buffer[0];

					if(sequence_count==4)
					{
						printf("Attempting to reset sequence id - image may be corrupt.\n");
						dscf55_sequence_id=0;

						while(dsc_sequence[dscf55_sequence_id] != old_sequence)
							dscf55_sequence_id++;

						return TRUE;
					}

					printf("Invalid Sequence\n");
					ps.buffer[0] = 129;
					ps.checksum = CalcCheckSum(&ps);
					break;

				case DSC_RESET_SEQUENCE:
					dscf55_sequence_id=0;
					return TRUE;
		
				case DSC_RESEND_PACKET:
					printf("Resending Packet\n");
					break;

				case TRUE:
					return TRUE;

				default:
					printf("Unknown Error\n");
					break;
			}
			else
			{
/*				printf("Incomplete packet\n"); */
				ps.buffer[0] = 129;
				ps.checksum = CalcCheckSum(&ps);
			}
	}

	printf("Failed to read packet.\n");
	exit(0);

	return FALSE;
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
	int sc=11; /* count of bytes to skip at start of packet */
	struct Image *image;
	FILE *temp;
	Packet dp;

	if(PORT_SPEED == B115200)
	{
		SetTransferRate[3] = 4;
		Converse(&dp, SetTransferRate, 4);
		dscSetSpeed(B115200);
		Converse(&dp, EmptyPacket, 1);
		usleep(50000);
	}

	if(thumbnail)
	{

		sc = 0x247;

		if(!Converse(&dp, StillImage, 19))
			printf("StillImage Failed\n");

		SelectImage[4] = imageid;
		Converse(&dp, SelectImage, 7);

		sprintf(filename, "/tmp/gphoto_image_%u.jpg", imageid-1);
		temp = fopen(filename, "wb");

		for(;;)
		{
			Converse(&dp, SendThumbnail, 4);

			fwrite((char *)dp.buffer+sc, sizeof(char), dp.length-sc, temp);
			sc=7;

			if(3==dp.buffer[4])
				break;
		}

		fclose(temp);
	}
	else
	{
		int last_packet = 0;

		sprintf(filename, "/tmp/gphoto_image_%u.jpg", imageid-1);

		if(!Converse(&dp, StillImage, 19))
			printf("StillImage Failed\n");

		temp = fopen(filename, "wb");

		SendImage[4] = imageid;
		Converse(&dp, SendImage, 7);


		for(;;)
		{
			fwrite((char *)dp.buffer+sc, sizeof(char), dp.length-sc, temp);
			sc=7;

			if(3==dp.buffer[4])
				break;

			Converse(&dp, SendNextImagePacket, 4);
		}

		fclose(temp);
	}

	fp = fopen(filename, "r");

	image = 0;

	if(fp)
	{
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		rewind(fp);
		image = (struct Image*)malloc(sizeof(struct Image));

		if(image)
		{
			image->image = (char*)malloc(sizeof(char)*size);
			image->image_size = size;
			image->image_info_size = 0;
			fread(image->image, sizeof(char), size, fp);
		}
		else
			printf("Failed to allocate Image struct\n");

		fclose(fp);
		unlink(filename);
	}
	else
		printf("Failed to open file\n");

	SetTransferRate[3] = 0;
	Converse(&dp, SetTransferRate, 4);
	dscSetSpeed(B9600);
	Converse(&dp, EmptyPacket, 1);

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

*/
int sony_dscf55_store_picture(int imageid, char *path)
{
	FILE *temp;
	int f;
	int sc=11; /* count of bytes to skip at start of packet */
	char filename[64];
	long size;
	Packet dp;

	if(PORT_SPEED == B115200)
	{
		SetTransferRate[3] = 4;
		Converse(&dp, SetTransferRate, 4);
		dscSetSpeed(B115200);
		Converse(&dp, EmptyPacket, 1);
	}

	SelectImage[4] = imageid;
	Converse(&dp, SelectImage, 7);

	sprintf(filename, "%s/%11.11s", path, dp.buffer+5);
	printf("%s\n", filename);

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

	if(!Converse(&dp, StillImage, 19))
		printf("StillImage Failed\n");

	temp = fopen(filename, "wb");
	printf("%s\n", filename);

	usleep(50000);

	SendImage[4] = imageid;
	Converse(&dp, SendImage, 7);

	for(;;)
	{
		fwrite((char *)dp.buffer+sc, sizeof(char), dp.length-sc, temp);
		sc=7;

		if(3==dp.buffer[3])
			break;

		Converse(&dp, SendNextImagePacket, 4);
	}

	fclose(temp);

	SetTransferRate[3] = 0;
	Converse(&dp, SetTransferRate, 4);
	dscSetSpeed(B9600);
	Converse(&dp, EmptyPacket, 1);

	return 1;
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

