/*
  Fuji Camera library for the gphoto project, 1998 Matthew G. Martin
  This routine works for DS-7 .  Don't know about any others.

  Reworked from the "fujiplay" package:
      * A program to control Fujifilm digital cameras, like
      * the DS-7 and MX-700, and their clones.
      *
      * $Id$
      *
      * Written by Thierry Bousch <bousch@topo.math.u-psud.fr>
      * and released in the public domain.

    Portions of this code were adapted taken from
    GDS7 v0.1 interactive digital image transfer software for DS-7 camera
    Copyright (C) 1998 Matthew G. Martin

    Some of which was derived from get_ds7 , a Perl Language library
    Copyright (C) 1997 Mamoru Ohno

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include "exif.h"
#include "../src/gphoto.h"
#include "../src/util.h"
#include "../src/callbacks.h"

/*#include "blank.xpm"*/

extern unsigned char *fuji_exif_convert(exifparser *exifdat);
extern GtkWidget *open_fuji_config_dialog();

#ifndef CLK_TCK
#include <sys/param.h>
#define CLK_TCK HZ
#endif

#if !defined(B57600) && defined(EXTA)
#define B57600 EXTA
#endif

#if !defined(B115200) && defined(EXTB)
#define B115200 EXTB
#endif

struct pict_info {
	char *name;
	int number;
	int size;
	short ondisk;
	short transferred;
};

int fuji_initialized=0;  /* MGM */
int fuji_count;               /* MGM */
int fuji_size;           /* MGM */
int fuji_piccount;       /* For unique filename to Defeat the ImLib Cache */
int devfd = -1;
int maxnum;
int fuji_debug=0;

struct termios oldt, newt;
char has_cmd[256];
int pictures;
int interrupted = 0;
int pending_input = 0;
struct pict_info *pinfo = NULL;

unsigned char answer[5000];
int answer_len = 0;

static int get_raw_byte (void)
{
	static unsigned char buffer[128];
	static unsigned char *bufstart;
	int ret;

	while (!pending_input) {
		/* Refill the buffer */
		ret = read(devfd, buffer, 128);
		if (ret == 0)
			return -1;  /* timeout */
		if (ret < 0) {
			if (errno == EINTR)
				continue;
			return -1;  /* error */
		}
		pending_input = ret;
		bufstart = buffer;
	}
	pending_input--;
	return *bufstart++;
}

int wait_for_input (int seconds)
{
	fd_set rfds;
	struct timeval tv;

	if (pending_input)
		return 1;
	if (!seconds)
		return 0;

	FD_ZERO(&rfds);
	FD_SET(devfd, &rfds);
	tv.tv_sec = seconds;
	tv.tv_usec = 0;

	return select(1+devfd, &rfds, NULL, NULL, &tv);
}

int get_byte (void)
{
	int c;

	c = get_raw_byte();
	if (c < 255)
		return c;
	c = get_raw_byte();
	if (c == 255)
		return c;	/* escaped '\377' */
	if (c != 0)
		fprintf(stderr, "get_byte: impossible escape sequence following 0xFF\n");
	/* Otherwise, it's a parity or framing error */
	get_raw_byte();
	return -1;
}

int put_bytes (int n, unsigned char* buff)
{
	int ret;

	while (n > 0) {
		ret = write(devfd, buff, n);
		if (ret < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		n -= ret;
		buff += ret;
	}
	return 0;
}

int put_byte (int c)
{
	unsigned char buff[1];

	buff[0] = c;
	return put_bytes(1, buff);
}

int attention (void)
{
	int i;

	/* drain input */
	while (get_byte() >= 0)
		continue;
	for (i = 0; i < 3; i++) {
		put_byte(0x05);
		if (get_byte() == 0x06)
			return 0;
	}
	update_status("The camera does not respond.");
	return(-1);
}

void send_packet (int len, unsigned char *data, int last)
{
	unsigned char *p, *end, buff[3];
	int check;

	last = last ? 0x03 : 0x17;
	check = last;
	end = data + len;
	for (p = data; p < end; p++)
		check ^= *p;

	/* Start of frame */
	buff[0] = 0x10;
	buff[1] = 0x02;
	put_bytes(2, buff);

	/* Data */
	for (p = data; p < end; p++)
		if (*p == 0x10) {
			/* Escape the last character */
			put_bytes(p-data+1, data);
			data = p+1;
			put_byte(0x10);
		}
	put_bytes(end-data, data);

	/* End of frame */
	buff[1] = last;
	buff[2] = check;
	put_bytes(3, buff);
}

int read_packet (void)
{
	unsigned char *p = answer;
	int c, check, incomplete;

	if (get_byte() != 0x10 || get_byte() != 0x02) {
bad_frame:
		/* drain input */
		while (get_byte() >= 0)
			continue;
		return -1;
	}
	check = 0;
	while(1) {
		if ((c = get_byte()) < 0)
			goto bad_frame;
		if (c == 0x10) {
			if ((c = get_byte()) < 0)
				goto bad_frame;
			if (c == 0x03 || c == 0x17) {
				incomplete = (c == 0x17);
				break;
			}
		}
		*p++ = c;
		check ^= c;
	}
	/* Append a sentry '\0' at the end of the buffer, for the convenience
	   of C programmers */
	*p = '\0';
	answer_len = p - answer;
	check ^= c;
	c = get_byte();
	if (c != check)
		return -1;
	if (answer[2] + (answer[3]<<8) != answer_len - 4)
		return -1;
	/* Return 0 for the last packet, 1 otherwise */
	return incomplete;
}


int cmd (int len, unsigned char *data, char *indata)
{
	int i, c, timeout=50;
	fuji_count=0;

	/* Some commands require large timeouts */
	switch (data[1]) {
	  case 0x27:	/* Take picture */
	  case 0x34:	/* Recharge the flash */
	  case 0x64:	/* Take preview */
	    timeout = 12;
	    break;
	  case 0x19:	/* Erase a picture */
	    timeout = 1;
	    break;
	}
		
	for (i = 0; i < 3; i++) {
		send_packet(len, data, 1);
		wait_for_input(timeout);
		c = get_byte();
		if (c == 0x06)
			goto rd_pkt;
		if (c != 0x15){
			if(attention()) return(-1);
		};
	}
	update_status( "Cannot issue command, aborting.");
	return(-1);
rd_pkt:
	wait_for_input(timeout);

	for (i = 0; i < 3; i++) {
		if (i) put_byte(0x15);
		c = read_packet();
		if (c < 0)
			continue;
		if (c && interrupted) {
			fprintf(stderr, "\nInterrupted!\n");
			exit(1);
		}
		put_byte(0x06);
		if (indata != NULL) {
		  memcpy(indata+fuji_count,answer+4,answer_len-4);
			fuji_count+=answer_len-4;
			if (fuji_debug){
			  printf("Recd %d of %d\n",fuji_count,fuji_size);
			};
			update_progress((1.0*fuji_count/fuji_size>1.0)?1.0:
						1.0*fuji_count/fuji_size);

		};
		/* More packets ? */
		if (c != 0)
			goto rd_pkt;
		update_progress(0); /* Clean up the indicator */
		return 0;
	}		
	fprintf(stderr, "Cannot receive answer, aborting.\n");
	return(-1);
}

int cmd0 (int c0, int c1, char* indata)
{
	unsigned char b[4];

	b[0] = c0; b[1] = c1;
	b[2] = b[3] = 0;
	return cmd(4, b, indata);
}

int cmd1 (int c0, int c1, int arg, char* indata)
{
	unsigned char b[5];

	b[0] = c0; b[1] = c1;
	b[2] =  1; b[3] =  0;
	b[4] = arg;
	return cmd(5, b, indata);
}

int cmd2 (int c0, int c1, int arg, char *indata)
{
	unsigned char b[6];

	b[0] = c0; b[1] = c1;
	b[2] =  2; b[3] =  0;
	b[4] = arg; b[5] = arg>>8;
	return cmd(6, b, indata);
}

char* dc_version_info (void)
{
	cmd0 (0, 0x09, NULL);
	return answer+4;
}

char* dc_camera_type (void)
{
	cmd0 (0, 0x29, NULL);
	return answer+4;
}

char* dc_camera_id (void)
{
	cmd0 (0, 0x80, NULL);
	return answer+4;
}

int dc_set_camera_id (const char *id)
{
	unsigned char b[14];
	int n = strlen(id);

	if (n > 10)
		n = 10;
	b[0] = 0;
	b[1] = 0x82;
	b[2] = n;
	b[3] = 0;
	memcpy(b+4, id, n);
	return cmd(n+4, b, NULL);
}

char* dc_get_date (void)
{
	char *fmtdate = answer+50;
	
	cmd0 (0, 0x84, NULL);
	strcpy(fmtdate, "YYYY/MM/DD HH:MM:SS");
	memcpy(fmtdate,    answer+4,   4);	/* year */
	memcpy(fmtdate+5,  answer+8,   2);	/* month */
	memcpy(fmtdate+8,  answer+10,  2);	/* day */
	memcpy(fmtdate+11, answer+12,  2);	/* hour */
	memcpy(fmtdate+14, answer+14,  2);	/* minutes */
	memcpy(fmtdate+17, answer+16,  2);	/* seconds */

	return fmtdate;
}

int dc_set_date (struct tm *pt)
{
	unsigned char b[50];

	sprintf(b+4, "%04d%02d%02d%02d%02d%02d", 1900 + pt->tm_year, pt->tm_mon+1, pt->tm_mday,
		pt->tm_hour, pt->tm_min, pt->tm_sec);
	b[0] = 0;
	b[1] = 0x86;
	b[2] = strlen(b+4);	/* should be 14 */
	b[3] = 0;
	return cmd(4+b[2], b, NULL);
}

int dc_get_flash_mode (void)
{
	cmd0 (0, 0x30, NULL);
	return answer[4];
}

int dc_set_flash_mode (int mode)
{
	cmd1 (0, 0x32, mode, NULL);
	return answer[4];
}

int dc_nb_pictures (void)
{
	if (cmd0 (0, 0x0b, NULL)) return(-1);
	return answer[4] + (answer[5]<<8);
}

char *dc_picture_name (int i)
{
	cmd2 (0, 0x0a, i, NULL);
	return answer+4;
}

int dc_picture_size (int i)
{
	cmd2 (0, 0x17, i, NULL);
	return answer[4] + (answer[5] << 8) + (answer[6] << 16) + (answer[7] << 24);
}

int charge_flash (void)
{
	cmd2 (0, 0x34, 200, NULL);
	return answer[4];
}

int take_picture (void)
{
	cmd0 (0, 0x27, NULL);
	return answer[4] + (answer[5] << 8) + (answer[6] << 16) + (answer[7] << 24);
}

int del_frame (int i)
{
	cmd2 (0, 0x19, i, NULL);
	return answer[4];
}

void get_command_list (void)
{
	int i;

	memset(has_cmd, 0, 256);
	if (cmd0 (0, 0x4c, NULL)) return ;
	for (i = 4; i < answer_len; i++)
	  has_cmd[answer[i]] = 1;
	/*		has_cmd[i] = 1;*/
}

int get_picture_info(int num,char *name){

          if(fuji_debug)  printf("Getting name...");

	  fflush(stdout);
	  strncpy(name,dc_picture_name(num),100);

	  if (fuji_debug) printf("%s\n",name);

	  /*
	   * To find the picture number, go to the first digit. According to
	   * recent Exif specs, n_off can be either 3 or 4.
	   */
	  if (has_cmd[0x17])   fuji_size=dc_picture_size(num);
	  else fuji_size=70000;  /* this is an overestimation for DS7 */
	  return (fuji_size);
};

void get_picture_list (void)
{
	int i, n_off;
	char *name;
	struct stat st;

	pictures = dc_nb_pictures();
	maxnum = 100;
	free(pinfo);
	pinfo = calloc(pictures+1, sizeof(struct pict_info));
	for (i = 1; i <= pictures; i++) {
	        if (fuji_debug) printf("Getting name..."); /* mgm1 debug*/
	        fflush(stdout);
	        name = strdup(dc_picture_name(i));
	        pinfo[i].name = name;

		if (fuji_debug) printf("%s\n",name); /* mgm1 debug*/

		/*
		 * To find the picture number, go to the first digit. According to
		 * recent Exif specs, n_off can be either 3 or 4.
		 */
		n_off = strcspn(name, "0123456789");
		if ((pinfo[i].number = atoi(name+n_off)) > maxnum)
			maxnum = pinfo[i].number;
		if (has_cmd[0x17])	pinfo[i].size = dc_picture_size(i);
		else pinfo[i].size=65000;
		pinfo[i].ondisk = !stat(name, &st);
	}
}

void list_pictures (void)
{
	int i;
	struct pict_info* pi;
	char ex;

	for (i = 1; i <= pictures; i++) {
		pi = &pinfo[i];
		ex = pi->ondisk ? '*' : ' ';
		printf("%3d%c  %12s  %7d\n", i, ex, pi->name, pi->size);
	}
}

void close_connection (void)
{
	put_byte(0x04);
	tcdrain(devfd);
	usleep(50000);
}

void reset_serial (void)
{
	if (devfd >= 0) {
		close_connection();
		tcsetattr(devfd, TCSANOW, &oldt);
	}
	devfd = -1;
}

int init_serial (const char *devname)
{
	devfd = open(devname, O_RDWR|O_NOCTTY);
	if (devfd < 0) {
		perror("Cannot open device");
		exit(1);
	}
	if (tcgetattr(devfd, &oldt) < 0) {
		perror("tcgetattr");
		exit(1);
	}
	newt = oldt;
	newt.c_iflag |= (PARMRK|INPCK);
	newt.c_iflag &= ~(BRKINT|IGNBRK|IGNPAR|ISTRIP|INLCR|IGNCR|ICRNL|IXON|IXOFF);
	newt.c_oflag &= ~(OPOST);
	newt.c_cflag |= (CLOCAL|CREAD|CS8|PARENB);
	newt.c_cflag &= ~(CSTOPB|HUPCL|PARODD);
	newt.c_lflag &= ~(ECHO|ECHOE|ECHOK|ECHONL|ICANON|ISIG|NOFLSH|TOSTOP);
	newt.c_cc[VMIN] = 0;
	newt.c_cc[VTIME] = 1;
	cfsetispeed(&newt, B9600);
	cfsetispeed(&newt, B9600);
	if (tcsetattr(devfd, TCSANOW, &newt) < 0) {
		perror("tcsetattr");
		exit(1);
	}
	atexit(reset_serial);
       return(attention());
}

void set_max_speed (void)
{
	int speed;

#ifdef B115200
	speed = B115200;
	cmd1(1, 7, 8, NULL);
	if (answer[4] == 0)
		goto change_speed;
#endif
#ifdef B57600
	speed = B57600;
	cmd1(1, 7, 7, NULL);
	if (answer[4] == 0)
		goto change_speed;
#endif
	speed = B38400;
	cmd1(1, 7, 6, NULL);
	if (answer[4] == 0)
		goto change_speed;

	/* Ugh -- we're stuck at 9600 bps */
	return;

change_speed:
	close_connection();
	cfsetispeed(&newt, speed);
	cfsetospeed(&newt, speed);
	tcsetattr(devfd, TCSANOW, &newt);
	attention();
}

int download_picture(int n,int thumb,struct Image *im)
{
	char name[100];
	/*	int size;*/
	struct stat st;
	clock_t t1, t2;

	if (!thumb) {
	        fuji_size=get_picture_info(n,name);

		if (fuji_debug) printf("%3d   %12s  \n", n, name);
	}
	else fuji_size=10500;  /* Probly not same for all cams, better way ? */
	im->image_size=fuji_size;
	im->image=malloc(fuji_size*2);  /* Provide some headroom */
	
	t1 = times(0);
	if (cmd2(0, (thumb?0:0x02), n, im->image)==-1) return(-1);

	if (fuji_debug) printf("Download %s: %4d actual bytes vs %4d bytes\n", 
			       (thumb?"thumbnail":"picture"),fuji_count , 
			       im->image_size);
	if (im->image_size>(fuji_count*2)) {
	  free(im->image);
	  im->image=NULL;
	  return(-1);
	};
	im->image_size=fuji_count;
	t2 = times(0);

	if (fuji_debug){
	        printf("%3d seconds, ", (int)(t2-t1) / CLK_TCK);
		printf("%4d bytes/s\n", fuji_count * CLK_TCK / (int)(t2-t1));
	};

	if (has_cmd[17]&&!thumb){
	if (!thumb&&(fuji_count != fuji_size)){
	    update_status("Short picture file--disk full or quota exceeded\n");
	    return(-1);
	  };
	};
	return(0);
}

int dc_free_memory (void)
{
	cmd0 (0, 0x1B, NULL);
	return answer[5] + (answer[6]<<8) + (answer[7]<<16) + (answer[8]<<24);
}


int delete_pic (const char *picname)
{
	int i, ret;

	for (i = 1; i <= pictures; i++)
	        if (!strcmp(pinfo[i].name, picname)) {
	                 if ((ret = del_frame(i)) == 0)
			          get_picture_list();
			 return ret;
		}
	return -1;
}


char* auto_rename (void)
{
	static char buffer[13];
	
	if (maxnum < 99999)
		maxnum++;

	sprintf(buffer, "DSC%05d.JPG", maxnum);
	return buffer;
}


int upload_pic (const char *picname)
{
	unsigned char buffer[516];
	const char *p;
	struct stat st;
	FILE *fd;
	int c, last, len, free_space;

	fd = fopen(picname, "r");
	if (fd == NULL) {
	  update_status("Cannot open file for upload");
		return 0;
	}
	if (fstat(fileno(fd), &st) < 0) {
		perror("fstat");
		return 0;
	}
	free_space = dc_free_memory();
	fprintf(stderr, "Uploading %s (size %d, available %d bytes)\n",
		picname, (int) st.st_size, free_space);
	if (st.st_size > free_space) {
		fprintf(stderr, "  not enough space\n");
		return 0;
	}
	if ((p = strrchr(picname, '/')) != NULL)
		picname = p+1;
	if (strlen(picname) != 12 || memcmp(picname,"DSC",3) || memcmp(picname+8,".JPG",4)) {
		picname = auto_rename();
		fprintf(stderr, "  file renamed %s\n", picname);
	}
	buffer[0] = 0;
	buffer[1] = 0x0F;
	buffer[2] = 12;
	buffer[3] = 0;
	memcpy(buffer+4, picname, 12);
	cmd(16, buffer, NULL);
	if (answer[4] != 0) {
		fprintf(stderr, "  rejected by the camera\n");
		return 0;
	}
	buffer[1] = 0x0E;
	while(1) {
		len = fread(buffer+4, 1, 512, fd);
		if (!len) break;
		buffer[2] = len;
		buffer[3] = (len>>8);
		last = 1;
		if ((c = getc(fd)) != EOF) {
			last = 0;
			ungetc(c, fd);
		}
		if (!last && interrupted) {
			fprintf(stderr, "Interrupted!\n");
			exit(1);
		}
again:
		send_packet(4+len, buffer, last);
		wait_for_input(1);
		if (get_byte() == 0x15)
			goto again;
	}
	fclose(fd);
	fprintf(stderr, "  looks ok\n");
	return 1;
}


int fuji_init(){
        char idstring[256];

	if (fuji_debug) printf("Initializing %s\n",serial_port);

	if (init_serial(serial_port)==-1) return(-1);

	set_max_speed();
	if (!fuji_initialized){
	        get_command_list();
	  /* For now assume that the user wil not use 2 different fuji cams
	     in one session */
	        strcpy(idstring,"Identified ");
	        strncat(idstring,dc_version_info(),100);
	        update_status(idstring);

	  /* load_fuji_options() */
	        fuji_initialized=1;  
    };
      
    return(0);
};


/***********************************************************************
     gphoto lib calls
***********************************************************************/


int fuji_configure(){
  /* 
     DS7 can't be configured, looks like other Fuji cams can...
     expansion needed here.
  */
 open_fuji_config_dialog();

  return(1);
};

struct Image *fuji_get_picture (int picture_number,int thumbnail){
  GdkImlibImage *imlibimage;
  char rm[1024],tmpfname[1024];
  unsigned char *buffer;
  struct Image *newimage;
  FILE *fd;
  int i;
  exifparser exifdat;


  if (fuji_debug) printf("fuji_get_picture called for #%d %s\n",picture_number,
			 thumbnail?"thumb":"photo");

  if (fuji_init()) return(0);/*goto bugout*/;

  newimage=malloc(sizeof(struct Image));
  newimage->image=NULL;

  if (thumbnail)
    sprintf(tmpfname, "%s/gphoto-thumbnail-%i-%i.jpg",
	    gphotoDir, getpid(),fuji_piccount);
  else {
    sprintf(tmpfname, "%s/gphoto-%i-%i.jpg", 
	    gphotoDir, getpid(), fuji_piccount);
  };
  fuji_piccount++;

  if (download_picture(picture_number,thumbnail,newimage)) {
    free(newimage);
    return(0);
  };

  buffer=newimage->image;

  /* Work on the tags...*/
  exifdat.header=buffer;
  exifdat.data=buffer+12;

  if (exif_header_parse(&exifdat)>0){
    stat_exif(&exifdat);
  
    newimage->image_info=
      malloc(sizeof(char*)*(exifdat.ifdtags[thumbnail?1:0]*2+(thumbnail?6:0)));
    if (newimage->image_info==NULL) {
      fprintf(stderr,"BIG TROUBLE!!! Bad image memory allocation\n");
      return(0);
    };
    newimage->image_info_size=exifdat.ifdtags[thumbnail?1:0]*2;

    if (thumbnail) {/* This SHOULD be done by tag id */
      togphotostr(&exifdat,0,0,newimage->image_info,newimage->image_info+1);
      togphotostr(&exifdat,0,1,newimage->image_info+2,newimage->image_info+3);
      togphotostr(&exifdat,0,2,newimage->image_info+4,newimage->image_info+5);
      newimage->image_info_size+=6;
    };

    exif_add_all(&exifdat,thumbnail?1:0,newimage->image_info+(thumbnail?6:0));

    if (fuji_debug) {
            printf("====================EXIF TAGS================\n");
	    for(i=0;i<newimage->image_info_size/2;i++)
	            printf("%12s = %12s \n",newimage->image_info[i*2],
			   newimage->image_info[i*2+1]);
	    printf("=============================================\n");
    };

    /* Tiff info in thumbnail must be converted to be viewable */
    if (thumbnail) {
      newimage->image=fuji_exif_convert(&exifdat);
      
      if (newimage->image==NULL) {
	FILE *fp;
	
	fprintf(stderr,"Thumbnail conversion error, saving data to \
fuji-death-dump.dat\n\
\tPlease mail this file to Matt.Martin@ieee.org along with a description \
of setup, camera model and any text output.\n");
	if ((fp=fopen("fuji-death-dump.dat","w"))!=NULL){
	  fwrite(buffer,1,newimage->image_size,fp);
	  fclose(fp);
	};
	free(newimage);
	return(0);
      };
      strcpy(newimage->image_type,"tif");
    }
    else  {
      newimage->image=buffer;
      strcpy(newimage->image_type,"jpg");
    };
  } else    { /* Wasn't recognizable exif data */
	FILE *fp;
	fprintf(stderr,"Saving problem data to fuji-death-dump.dat\n");
	if ((fp=fopen("fuji-death-dump.dat","w"))!=NULL){
	  fwrite(newimage->image,1,newimage->image_size,fp);
	  fclose(fp);
	};
  };

  reset_serial(); /* Wish this wasn't necessary...*/
  return(newimage);

  /*
bugout:
  imlibimage = gdk_imlib_create_image_from_xpm_data(blank_xpm);
  newimage->image=(char *)blank_xpm;
  reset_serial();
  return(newimage);
*/
};

int fuji_delete_image (int picture_number){
  char picname[100];
  if (!has_cmd[19]) {
    return(0);
  };
  /*  if (get_picture_info(picture_number,picname)) return(-1);
      return(delete_pic (picname));*/
  /* Warning, does the index need to be re-counted after each delete?*/
  return(del_frame(picture_number));
};

int fuji_number_of_pictures (){
  int numpix;

  if (fuji_init()) return (-1);
  numpix=dc_nb_pictures();

  reset_serial(); /* Wish this wasn't necessary...*/
  return(numpix);

};

int fuji_initialize () {

	return 1;
}

struct Image *fuji_get_preview (){
  char tmpfname[1024],rm[1024];
  FILE *fd;

  struct Image *newimage;

  newimage=malloc(sizeof( struct Image));

  if (!has_cmd[0x62] || !has_cmd[0x64]) {
    update_status("Cannot preview (unsupported command)\n");
    /*    newimage->image=(char *)blank_xpm;*/
    return ( 0 );
  }

  newimage->image=malloc(100000); /* What a hack!!!*/

  cmd0(0, 0x64, NULL);
  cmd0(0, 0x62, newimage->image);

  return(newimage);
};

int fuji_take_picture (){
  if (!has_cmd[0x62] || !has_cmd[0x64]) {
    update_status("Cannot take picture (unsupported command)\n");
    return(0);
  }
  return(  take_picture());
};

char *fuji_summary() {
	return ("Not Yet Supported.");
}

char *fuji_description() {
	return(
"Fuji DS-7
Matthew G. Martin
Based on fujiplay by
Thierry Bousch<bousch@topo.math.u-psud.fr>

Known to work with Fuji DS-7 camera, but 
may support other Fuji cameras.");
}

struct _Camera fuji = {fuji_initialize,
		       fuji_get_picture,
		       fuji_get_preview,
		       fuji_delete_image,
		       fuji_take_picture,
		       fuji_number_of_pictures,
		       fuji_configure,
		       fuji_summary,
		       fuji_description
};

