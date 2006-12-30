/****************************************************************************
 * rdc2e - program to download pictures from a Ricoh RDC-2E digital camera
 ****************************************************************************
 *
 *            Absolutely no warranty.
 *            This software is coverd by th GNU Public License (GPL)
 *            Read the COPYING file that comes with the distribution.
 *
 ****************************************************************************/
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <stdio.h>
#include <errno.h>
#include "crctab.c"
#include "config.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#ifndef CAMERAPORT
#define CAMERAPORT      "/dev/ttyS1"
#endif

#define	ESCAPE_CHAR	'\020'
#define START_FRAME	'\002'
#define END_FRAME	'\003'
#define ACK		'\005'
#define NAK		'\017'
#define RETRIES		10
#define DATAREAD	1
#define RAWREAD		0


/*
 * Function prototypes
 */
void	init_port();
void	close_port();
int	read_frame();
int	read_char();
void	send_data();
void	send_frame();
void	send_ack();
void	send_nak();
int	receive_ack();
void	set_speed();
int	query_number_of_pictures();
void	hex_dump();
void	download_range();
void	download_picture();
void	send_download_picture();
int	read_picture_info();
void	read_whole_picture();
void	name_to_lowercase();
void	show_help();
void	ShowDate();
void	WWWserver();
void	WWWgetpicture();
void	WWWshowlist();


/*
 * Global variables
 */
int	debug=0;
struct	termios	OldTermios, NewTermios;
int	CameraPort, PictureFile;
FILE	*screen;
char	hash_char;

/************************************************************************
 * main()
 ************************************************************************
 *
 * All evil starts from here...
 *
 ************************************************************************/
int main(int argc, char **argv)
{
int	number_of_pictures;

int	optchar;
extern	char	*optarg;
extern	int	optind, opterr, optopt;
char	opts[20];
char	speed[30], cameraport[40], prefix[100], FinishedCommand[256];
char	*range;

	screen=stdout;
	strcpy((char *)&opts,"vhdqs:i:p:l:f:t:w");
	strcpy((char *)&speed,"115200");
	strcpy((char *)&cameraport,CAMERAPORT);
	prefix[0]=0;
	FinishedCommand[0]=0;

	while (1) {
		optchar=getopt(argc, argv, (char *)&opts);
		if (optchar == EOF)
			break;
		switch (optchar) {
			case 'd':
				debug++;
				break;
			case 'i':
				strcpy((char *)&cameraport,optarg);
				break;
			case 'p':
				strcpy((char *)&prefix,optarg);
				break;
			case 'q':
				screen=fopen("/dev/null","w");
				break;
			case 's':
				strcpy((char *)&speed,optarg);
				break;
			case 'f':
				strcpy((char *)&FinishedCommand,optarg);
				break;
			case 't':
				ShowDate(optarg);
				exit(0);
				break;
			case 'v':
				printf("%s version 0.7 23 December 1999 by ",argv[0]);
				printf("Brian Miller <bmiller@netspace.net.au>\n");
				exit(0);
				break;
			case 'w':
				init_port(cameraport);                                                  	set_speed(speed);
				WWWserver();
				close_port();
				exit(0);
				break;
			case ':':
			case '?':
			case 'h':
				show_help();
				break;
		}
	}
	init_port(cameraport);

	set_speed(speed);

	number_of_pictures = query_number_of_pictures();
	if (debug) printf("There are %d pictures in memory\n",number_of_pictures);

	if (number_of_pictures) {
		fprintf(screen,"Total of %d pictures in camera.\n",number_of_pictures);
		fprintf(screen,"            ");
		fprintf(screen,"0%%                             50%%                           100%%\n");
		fprintf(screen,"Num Size    ");
		fprintf(screen,"|-------------------------------|-------------------------------|\n");
	}
	else {
		fprintf(stderr,"No pictures to download\n");
		return(0);
	}

	if (optind == argc) {
		if (debug)
			printf("Downloading ALL pictures.\n");
		download_range(1,number_of_pictures,number_of_pictures,prefix);
	}
	else {
		while (optind < argc) {
			range=(char *)strchr(argv[optind],'-');
			if (range)
				download_range(atoi(argv[optind]), atoi(range+1),
						number_of_pictures,prefix);
			else
				download_range(atoi(argv[optind]), atoi(argv[optind]),
						number_of_pictures,prefix);
			optind++;
		}
	}

	close_port();

	if (strlen(FinishedCommand)) {
		system(FinishedCommand);
	}
	
	return(0);
}

/************************************************************************
 * download_range()
 ************************************************************************
 *
 * This function initiates a download of a range of pictures.
 *
 ************************************************************************/
void	download_range(int start, int end, int total, char *prefix)
{
int	i;

	if (start == 0)
		start=1;
	if (end == 0 || end > total)
		end=total;

	for (i=start; i<=end; i++)
		download_picture(i,prefix);
}


/************************************************************************
 * download_picture()
 ************************************************************************
 *
 * This function starts of a download of a picture by number.
 *
 ************************************************************************/
void	download_picture(int pic_num, char *pic_prefix)
{
int	size;

	if (debug)
		printf("Entering download_picture() for pic %d with prefix %s\n",
			pic_num, pic_prefix);

	fprintf(screen,"%3d ",pic_num);
	usleep(250000);
	send_download_picture(pic_num);
	size = read_picture_info(pic_prefix,pic_num);
	read_whole_picture(size);
	if (debug) {
		fprintf(screen,"Finished downloading picture %d.\n",pic_num);
		fflush(screen);
	}

}

/************************************************************************
 * init_port()
 ************************************************************************
 *
 * This function opens the serial port & sets the initial speed to 
 * 2400 bps.
 *
 ************************************************************************/
void	init_port(char *port)
{
	if (debug) printf("Opening serial port %s... ",port);
	fflush(stdout);
	CameraPort = open(port, O_RDWR | O_NOCTTY );
	if (debug) printf(" done.\n");
	if (debug) fflush(stdout);
	if (CameraPort < 0) {
		perror("Unable to open camera serial port.");
		close_port();
		exit(-1);
	}
	
	tcgetattr(CameraPort,&OldTermios);
	
	bzero(&NewTermios,sizeof(NewTermios));
	
	NewTermios.c_iflag = IGNBRK | IGNPAR;
	NewTermios.c_oflag = 0;
	NewTermios.c_cflag = CS8 | CREAD | CRTSCTS | CLOCAL ;
	NewTermios.c_lflag = 0;
	
	cfsetispeed(&NewTermios,B2400);
	cfsetospeed(&NewTermios,B2400);
	
	NewTermios.c_cc[VTIME] = 10;
	NewTermios.c_cc[VMIN] = 0;
	
	tcflush(CameraPort,TCIOFLUSH);
	
	if (tcsetattr(CameraPort,TCSANOW,&NewTermios)) {
		perror("Unable to set interface with tcsetattr().");
		close_port();
		exit(-1);
	}

	if (debug) printf("tcsetattr() done\n");
}

/************************************************************************
 * close_port()
 ************************************************************************
 *
 * This function closes the serial port & returns it to its previous
 * settings.
 *
 ************************************************************************/
void	close_port()
{
	/*
	 * force the port to lower modem control lines so camera
         * will go offline.
	 */
	OldTermios.c_cflag = OldTermios.c_cflag | HUPCL ;

	tcsetattr(CameraPort,TCSANOW,&OldTermios);
	tcflush(CameraPort,TCIOFLUSH);
	close(CameraPort);
}

/************************************************************************
 * read_frame()
 ************************************************************************
 *
 * This function reads in a frame starting with 0x10 0x02 ....
 * and finishing with 0x10 0x03
 *
 * read_buf is a pointer to a char string that the frame data is returned in
 *	    This does not include the frame headers, CRC, length, etc.
 *
 * frame_sequence is a pointer to a single unsugned byte that is the 
 *          frame sequence number on the very end of the frame.
 *
 * RETURNS: The length of read_buf; -1 on error.
 ************************************************************************/

int	read_frame(unsigned char *read_buf, unsigned char *frame_sequence)
{
int	data_length, retries;
unsigned char	one_char;
unsigned char	frame_data_length;
unsigned short	calc_crc, read_crc;
unsigned char	*char_crc;

	if (debug) printf("Entering read_frame()\n");

	retries = RETRIES;
	while (retries--) {

		/*
		 * read up to & including start of frame i.e. the 0x10 0x02
		 */
		while (read_char(&one_char,DATAREAD) != 2) {}
	
		/*
		 * now sitting at start of data section of frame 
		 * can zap through and wait until end of frame
		 * i.e. until read_char returns a value of '3'
		 * may as well calculate the CRC while doing this too
		 */
		data_length = 0;
		calc_crc = 0;
		while (read_char(&one_char,DATAREAD) != 3) {
			read_buf[data_length] = one_char;
			calc_crc = updcrc((read_buf[data_length] & 0xff), calc_crc);
			data_length++;
		}
	
		/*
		 * now have read the 0x10 0x03 that is end of frame
		 * next is the two byte CRC.
		 * This is a short int in Intel byte order.
		 * If this is EVER ported to a non-Intel platform some fixes are needed
		 */
		char_crc = (unsigned char*)&read_crc;
		read_char(&one_char,RAWREAD);
		char_crc[0]=one_char;
		read_char(&one_char,RAWREAD);
		char_crc[1]=one_char;
	
		/*
		 * next to read is the single byte data length
		 */
		read_char(&frame_data_length,RAWREAD);
	
		/*
		 * last byte to read is the frame sequence number
		 */
		read_char(frame_sequence,RAWREAD);
		if (debug > 1) printf("\n");
	
		/*
		 * Check that the CRCs are a match & data langths match
		 */
		if (read_crc != calc_crc || (int)frame_data_length != data_length) {
			fprintf(screen,"%c",7);	/* warning with a beep too */
			if (read_crc != calc_crc)
				hash_char='c';
			else
				hash_char='l';
			if (debug)
				fprintf(screen,"retries=%d",retries);
			send_nak();
		}
		else {
			/*
		 	* Send an ack for the frame
		 	*/
			send_ack();
	
			/*
		 	* the end, return the length of the data segment
		 	*/
			if (debug)
				fprintf(screen,"frame_sequence: %02x, data_length: %d\n",
					(int)frame_sequence, data_length);
			return data_length;
		}
	}

	/* if here then retries on CRC has expired */
	close_port();
	fprintf(stderr,"Error reading valid frame from camera\n");
	exit(-1);

}

/************************************************************************
 * read_char()
 ************************************************************************
 *
 * This reads in a single char from the camera.
 *
 * If there is a timeout, this function ends with an error mesage.
 *
 * RETURNS: 0 normally.
 *          n if the character is a special begin or end of frame
 *            i.e. '2' if it's a start of frame or '3' for end of frame.
 ************************************************************************/

int read_char(unsigned char *c, int data_read_flag)
{
int retries;

	retries=RETRIES;

	while (retries && read(CameraPort,c,1) < 1) {
printf("RETRY(%d) ",retries);
		retries--;
	}

	if (!retries) {
		perror("Timeout reading a character from camera");
		close_port();
		exit(-1);
	}

	if (debug > 2) {
		printf("%02x ",c[0]);
		fflush(stdout);
	}

	if (c[0] == ESCAPE_CHAR && data_read_flag) {
		retries=RETRIES;
		while (retries && read(CameraPort,c,1) < 1) {
			retries--;
		}
	}
	else	/* if here, wasn't an escaped char */
		return 0;

	if (!retries) {
		perror("Timeout reading a character from camera");
		close_port();
		exit(-1);
	}
	
	if (debug > 1) printf("%02x ",c[0]);

	if (c[0] == START_FRAME)
		return 2;
	else if (c[0] == END_FRAME)
		return 3;
	else	/* probably just escaping a 0x10 in the data */
		return 0;
}

/************************************************************************
 * send_data()
 ************************************************************************
 *
 * This function pads out "data" for being placed into a frame with
 * the necessary 0x10s, then wraps it into a full frame to send to the
 * camera.
 *
 ************************************************************************/

void send_data(unsigned char *data, int data_length, int frame_count)
{

short int	crc;
unsigned char	frame[512];
int		i,possy;

	if (debug)
		printf("Entering send_data() with %d bytes for frame number %d...\n",data_length,frame_count);
	if (debug > 1)
		hex_dump(data,data_length);
	/*
	 * The maximum frame count I have seen is 0x82 (130 decimal)
	 */
	if (data_length > 130) {
		fprintf(stderr,"Data length greater than 130!\n");
		close_port();
		exit(-1);
	}

	possy=0;
	/*
	 * put in the header...
	 */
	frame[possy++]= ESCAPE_CHAR;
	frame[possy++]= START_FRAME;

	/*
	 * put in the data, looking for 0x10s in there
	 * and calculate the crc while doing it too.
	 */
	crc=0;
	for (i=0;i<data_length;i++) {
		crc = updcrc((data[i] & 0xff), crc);
		if (data[i] == ESCAPE_CHAR) 
			frame[possy++]=ESCAPE_CHAR;
		frame[possy++]=data[i]&0xff;
	}

	/*
	 * Now drop on the 0x10 0x03
	 */
	frame[possy++]= ESCAPE_CHAR;
	frame[possy++]= END_FRAME;

	/* 
	 * drop the CRC on the end
	 */
	memcpy(frame+possy,(char *)&crc,2);
	possy +=2;

	/* 
	 * single byte length field of the data segment
	 */
	frame[possy++] = (unsigned char)data_length;

	/*
	 * Single byte frame counter is last field
	 */
	frame[possy++]=(unsigned char)frame_count;

	/*
	 * now send this to the camera
	 */
	 send_frame(frame,possy);
}

/************************************************************************
 * send_frame()
 ************************************************************************
 *
 * This sends a pre-built frame to the camera, and waits for 
 * an ACK.
 *
 ************************************************************************/
void send_frame(unsigned char *frame, int length)
{

int	retries, sent_len;

	if (debug)
		printf("Entering send_frame() with %d bytes...\n",length);
	if (debug > 1)
		hex_dump(frame,length);

	retries=RETRIES;

	do {
		if (retries != RETRIES)
			usleep(500000);
		sent_len=write(CameraPort,frame,length);
		if (sent_len != length) {
			perror("Sending frame to camera");
			close_port();
			exit(-1);
		}
		retries--;
	} while (receive_ack() == 0 && retries > 0);

	if (retries < 1) {
		fprintf(stderr,"No response from camera.\n");
		close_port();
		exit(-1);
	}
}

/************************************************************************
 * void send_ack()
 ************************************************************************
 *
 * Send an ACK 0x05
 *
 ************************************************************************/
void send_ack()
{
unsigned char ack;

	ack = ACK;

	if (write(CameraPort,&ack,1) != 1) {
		perror("Error in writing ACK to camera");
		close_port();
		exit(-1);
	}
}

/************************************************************************
 * void send_nak()
 ************************************************************************
 *
 * Send an NAK 
 *
 ************************************************************************/
void send_nak()
{
unsigned char nak;

	nak = NAK;

	if (write(CameraPort,&nak,1) != 1) {
		perror("Error in writing NAK to camera");
		close_port();
		exit(-1);
	}
}

/************************************************************************
 * receive_ack()
 ************************************************************************
 *
 * Receive an ACK 0x05
 *
 * RETURNS: 0 on failure,
 *	    1 on success.
 ************************************************************************/
int receive_ack()
{
unsigned char ack;
int	retries;

	if (debug) printf("Entering receive_ack()");

	retries=RETRIES;

	while (retries && read(CameraPort,&ack,1) < 1) {
		if (debug) printf(".");
		retries--;
	}

	if (ack != ACK && debug) {
		fprintf(stderr,"Invalid ACK of value 0x%02X received.\n",ack&0xff);
		fprintf(stderr,"I have not seen a negative ACK, this may be one...\n");
	}

	if (retries) {
		if (debug) printf(" ACK received\n");
		return 1;
	}
	else {
		if (debug) printf(" _NO_ ACK received\n");
		return 0;
	}
}

/************************************************************************
 * set_speed()
 ************************************************************************
 *
 * Set serial port speed
 *
 ************************************************************************/
void set_speed(char *speed) 
{
unsigned char	speed_data[4];

	if (debug) printf("Setting speed to: %s\n",speed);

	speed_data[0] = 0x31 & 0xff;
	speed_data[1] = 0x00 & 0xff;
	speed_data[2] = 0x00 & 0xff;

	if (!strcmp(speed,"2400")) {
		cfsetispeed(&NewTermios,B2400);
		cfsetospeed(&NewTermios,B2400);
		speed_data[3] = 0x00 & 0xff;
	}
	else if (!strcmp(speed,"4800")) {
		cfsetispeed(&NewTermios,B4800);
		cfsetospeed(&NewTermios,B4800);
		speed_data[3] = 0x01 & 0xff;
	}
	else if (!strcmp(speed,"9600")) {
		cfsetispeed(&NewTermios,B9600);
		cfsetospeed(&NewTermios,B9600);
		speed_data[3] = 0x02 & 0xff;
	}
	else if (!strcmp(speed,"19200")) {
		cfsetispeed(&NewTermios,B19200);
		cfsetospeed(&NewTermios,B19200);
		speed_data[3] = 0x03 & 0xff;
	}
	else if (!strcmp(speed,"38400")) {
		cfsetispeed(&NewTermios,B38400);
		cfsetospeed(&NewTermios,B38400);
		speed_data[3] = 0x04 & 0xff;
	}
	else if (!strcmp(speed,"57600")) {
		cfsetispeed(&NewTermios,B57600);
		cfsetospeed(&NewTermios,B57600);
		speed_data[3] = 0x05 & 0xff;
	}
	else if (!strcmp(speed,"115200")) {
		cfsetispeed(&NewTermios,B115200);
		cfsetospeed(&NewTermios,B115200);
		speed_data[3] = 0x07 & 0xff;
	}
	else {
		fprintf(stderr,"Invalid speed %s selected.\n",speed);
		close_port();
		exit(-1);
	}

	/*
	 * send this data of the be packaged in a frame & sent.
	 */
	send_data(speed_data,4,0);

	/*
	 * now set serial speed to new speed
	 */
	if (tcsetattr(CameraPort,TCSANOW,&NewTermios)) {
		perror("Unable to set interface speed with tcsetattr().");
		close_port();
		exit(-1);
	}

	/*
	 * Wait 1 second for things to settle before continuing 
	 */
	sleep(1);
}

/************************************************************************
 * query_number_of_pictures()
 ************************************************************************
 *
 * Query the number of pictures from the camera. This may need parameters
 * later when I get a PCMCIA flash card???
 *
 * RETURNS: number of pictures present in camera.
 *
 ************************************************************************/
int query_number_of_pictures()
{
unsigned char	query_data[2];
unsigned char	sequence;
unsigned char	response_data[1024];
short int	num_pics;

	if (debug) printf("Entering query_number_of_pictures()\n");

	/*
	 * Query is just 0x49 0x00
	 */
	query_data[0] = 0x49 & 0xff;
	query_data[1] = 0x00 & 0xff;

	/*
	 * Send out the query
	 */
	send_data(query_data,2,0);

	/*
	 * get back the response
	 */
	if (read_frame(response_data,&sequence) != 7) {
		fprintf(stderr,"response to number of pictures is not 7 bytes long\n");
		close_port();
		exit(-1);
	}

	/*
	 * copy the short int out of the response data which is the
	 * number of pictures in the camera.
	 */
	memcpy((char *)&num_pics,response_data+3,2);

	return (int)num_pics;
}

/************************************************************************
 * hex_dump();
 ************************************************************************
 *
 * Do a hex of 'n' bytes of data...
 *
 ************************************************************************/
void hex_dump(char *data, int data_length)
{
int	i;

	for (i=0;i<data_length;i++) 
		printf("%02x ",data[i]&0xff);
	printf("\n");
}

/************************************************************************
 * send_download_picture();
 ************************************************************************
 *
 * Download a single picture by it's number
 *
 ************************************************************************/
void send_download_picture(int picture_number)
{
char	data[4];
short int	pic;

	if (debug) printf("Entering send_download_picture() for picture number: %d\n",picture_number);

	data[0]=0x40 & 0xff;
	data[1]=0x00 & 0xff;

	pic = picture_number;

	memcpy(data+2,(char *)&pic,2);
	send_data(data,4,0);

}

/************************************************************************
 * read_picture_info()
 ************************************************************************
 *
 * The next frame to read contains the pictures name from the camera.
 *
 ************************************************************************/
int read_picture_info(char *prefix, int pic_num)
{
char	data[1024], picture_name[20];
int	data_length, file_size, retries;
unsigned char sequence_number;
short	file_blocks;

	if (debug) printf("Entering read_picture_info()\n");

	retries=RETRIES;

	do {
		data_length=read_frame(data,&sequence_number);
		retries--;
	} while (data_length != 18 && retries);

	if (data_length != 18)
		return(-1);

	if (prefix[0]) {
		sprintf(picture_name,"%s%03d.j6i",prefix,pic_num);
	}
	else {
		/* file name is null terminated, so strcpy would also work */
		strncpy(picture_name,data+2,19);
		name_to_lowercase(picture_name);
	}

	memcpy((char *)&file_blocks,data+15,2);
	file_size=file_blocks*256;

	if (debug) printf("Picture name is: %s\n",picture_name);

	if (file_blocks == 0x0300) fprintf(screen,"Fine    ");
	else if (file_blocks == 0x0180) fprintf(screen,"Normal  ");
	else if (file_blocks == 0x00c0) fprintf(screen,"Economy ");
	else printf("(?) \n");

	PictureFile=open(picture_name,O_WRONLY | O_CREAT | O_EXCL , S_IRUSR | S_IWUSR);
	if (PictureFile == -1) {
		printf("\n");
		fflush(stdout);
		perror("Unable to open file");
		close_port();
		exit(-1);
	}
	return(file_size);
}

/************************************************************************
 * read_whole_picture()
 ************************************************************************
 *
 * Just sit here reading off the data as it comes in
 *
 ************************************************************************/
void read_whole_picture(int file_size)
{
char	data[1024];
unsigned char	expected_sequence_number, sequence_number;
int	data_length,cumulative_file_size, prev_hash_size, hash_size;

	if (debug) printf("Entering read_whole_picture()\n");

	expected_sequence_number = 1;
	cumulative_file_size=0;
	prev_hash_size=0;
	hash_char='#';

	do {
		data_length=read_frame(data,&sequence_number);
		if (debug)
			printf("Received frame %02x:\n",sequence_number);
		if (debug > 1)
			hex_dump(data,data_length);
		if (sequence_number != expected_sequence_number) {
			fprintf(stderr,"Out of sequence frames; exiting now.\n");
			fprintf(stderr,"expected_sequence_number: %02x  sequence_number: %02x\n",
				expected_sequence_number,sequence_number);
			close_port();
			exit(-1);
		}
		expected_sequence_number++;
		write(PictureFile,data+2,data_length-2);
		cumulative_file_size += data_length-2;
		hash_size=cumulative_file_size*65/file_size;
		if (hash_size > prev_hash_size) {
			fprintf(screen,"%c",hash_char);
			hash_char='#';
			fflush(stdout);
			prev_hash_size=hash_size;
		}
		if (debug)
			fprintf(screen,"file_size: %d  cumulative_file_size: %d  data_length: %d  hash_size: %d  prev_hash_size: %d\n", file_size, cumulative_file_size, data_length, hash_size, prev_hash_size);
	} while (data[1]&0x80); /* while the MORE bit is set */
	fprintf(screen,"\n"); fflush(stdout);
	close(PictureFile);
}

/************************************************************************
 * name_to_lowercase()
 ************************************************************************
 *
 * The filenames stored in the camera are in UPPER CASE, but I prefer
 * them in lower case.
 *
 ************************************************************************/
void name_to_lowercase(char *name)
{
	while (name[0]) {
		if (name[0] >= 'A' && name[0] <= 'Z')
			name[0] = name[0] - 'A' + 'a';
	name++;
	}
}

/************************************************************************
 * show_help()
 ************************************************************************
 *
 * SHow a basic help screen
 *
 ************************************************************************/
void show_help()
{
	printf("Usage: rdc2e [-d] [-i interface] [-p prefix] [-s speed]
             [-t image.j6i] [image-list]

OPTIONS
       -d     Turn on (verbose) debugging.
 
       -f finished_command
              Specify  a  command  that will be executed when the
              last image has been downloaded from the camera.  If
              the  command  contains  spaces,  then  it  must  be
              enclosed in quotes. This is useful to automatically
              start  the  j6iextract  command, or play a sound to
              signal that all downloading has been completed.
 
       -i interface
              Specify which serial port the  camera  is  on.  The
              default is /dev/ttyS1.
 
       -p prefix
              Specify  a  prefix  for  each image downloaded. The
              full file name will be the  prefix  followed  by  a
              three  digit number of the image, and a .j6i exten-
              sion.  If no prefix is specified, then the internal
              name  of the image in the camera will be used. This
              is r0100001.j6i, r0100002.j6i, ... r0100nnn.j6i
 
       -s speed
              Specify the serial line download speed in bits  per
              second.  It  has  to  be  one  of 2400, 4800, 9600,
              19200, 38400, 57600  or  115200.   The  default  is
              115200.
 
       -t image.j6i
              Print  the date and time the J6I image was taken by
              reading it from the image.j6i J6I image file.                                        
 
ARGUMENTS
       The only arguments are an optional image-list that  speci-
       fies  which  images  should be downloaded. These are space
       separated single  image  numbers,  or  ranges  of  numbers
       denoted as <n1>-<n2>.                                                                       
");
	exit(-1);
}

/************************************************************************
 * ShowDate
 ************************************************************************
 *
 * Show the date & time a picture was taken as stored in a J6I file.
 *
 ************************************************************************/
void	ShowDate(char *j6iName)
{
char	hex_date[7];
FILE	*j6i_file;

	j6i_file=fopen(j6iName,"r");
	if ((int)j6i_file < 1) {
		perror("Unable to open file");
		exit(-1);
	}

	if (fseek(j6i_file,68,SEEK_SET)) {
		perror("Unable to skip forward in J6I file");
		exit(-1);                                                       	}

	if (!fgets(hex_date,7,j6i_file)) {
		perror("Unable to read date from J6I file");
		exit(-1);                                                       	}

	printf("%02x/%02x/%02x %02x:%02x:%02x\n",
		hex_date[2]&0xff, hex_date[1]&0xff, hex_date[0]&0xff,
		hex_date[3]&0xff, hex_date[4]&0xff, hex_date[5]&0xff);

}

/************************************************************************
 * WWWserver
 ************************************************************************
 *
 * This is used so that rdc2e can be used by a WWW browser
 *
 ************************************************************************/
void	WWWserver()
{
char	line[1024];
int	picture_number;

	if (debug) printf("Entering WWWserver()\n");

	do {
		fgets((char *)&line,1023,stdin);
		if (debug) printf("line: %s\n",line);
	} while (!strstr((char *)&line,"GET "));

	picture_number=atoi((char *)(strstr((char *)&line,"GET ") + 5));

	if (picture_number) {
		WWWgetpicture(picture_number);
	}
	else {
		WWWshowlist();
	}
}

/************************************************************************
 * WWWgetpicture
 ************************************************************************
 *
 * This is used so that rdc2e can be used by a WWW browser
 * to retrieve a single image from the camera.
 *
 ************************************************************************/
void WWWgetpicture(int picture_number)
{
char	pic_name[50],pic_prefix[45], c;
int	raw_image, byte_count;

	sprintf(pic_prefix,"/tmp/rdc2e-%d-",getpid());

	download_picture(picture_number,pic_prefix);

	sprintf(pic_name,"%s%03d.j6i",pic_prefix,picture_number);

	raw_image=open(pic_name,0);
	if (raw_image<0) {
		printf("ERROR opening J6I file: %s",pic_name);
		exit(-1);
	}

	/* printf("Content-Type: image/jpeg\n\n"); */

	byte_count=0;

	while (read(raw_image, (char *)&c, 1)) {
		byte_count++;
		if (byte_count>172)
			printf("%c",c);
	}

	close(raw_image);
	unlink(pic_name);
	

}

/************************************************************************
 * WWWshowlist
 ************************************************************************
 *
 * This is used so that rdc2e can be used by a WWW browser
 * to list the images in the camera.
 *
 ************************************************************************/
void WWWshowlist()
{
int	number_of_pictures, i;

	number_of_pictures = query_number_of_pictures();

	printf("<html><head><title>rdc2e - Image List</title></head>\n");
	printf("<body><h1>rdc2e - Image List</h1>\n");

	if (number_of_pictures) {
		printf("Click on a link below to download the image:<p>\n");
		printf("<blockquote>\n");
	
		for (i=1;i<=number_of_pictures;i++)
			printf("<a href=/%d.jpg>Image %d</a><br>",i,i);
		printf("</blockquote>\n");
	}
	else {
		printf("<b>There are no images available to download.</b>\n");
	}
	printf("</body></html>\n");
}
