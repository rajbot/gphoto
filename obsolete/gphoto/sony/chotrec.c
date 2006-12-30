/* this code not complete */
#include "config.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#ifdef BINARYFILEMODE
#include <fcntl.h>  /* for setmode() */
#endif
#include <stdlib.h>
#include <time.h>
#if HAVE_UNISTD_H
# include <sys/types.h>
# include <sys/stat.h>
# include <unistd.h>
#endif
#if HAVE_SYS_PARAM_H
# include <sys/param.h>
#else
#define MAXPATHLEN 256
#endif
#include "common.h"
#include "command.h"
#ifdef X68
#include "tty_x68.h"
#else
#ifdef WIN32
#include "tty_w32.h"
#include "getopt.h"
#else
#ifdef OS2
#include "tty_os2.h"
#else
#ifdef DOS
#include "tty_dos.h"
#else
#include "tty.h"
#include <termios.h>
#endif /* DOS */
#endif /* OS2 */
#endif /* WIN32 */
#endif /* X68 */

#include "common.h"

#define MAX_PICTURE_NUM 108

#ifdef BINARYFILEMODE
#define WMODE "wb"
#define RMODE "rb"
#else
#define WMODE "w"
#define RMODE "r"
#endif

extern int	optind, opterr;
extern char	*optarg;

int	verbose = 0;
static int     speed = 0;
static int format = JPEG;

static	int	errflg = 0;
static	int	all_pic_num = -1;
#ifndef DONTCAREUID
static	uid_t	uid, euid;
static	gid_t	gid, egid;
static	int	uidswapped = 0;
#endif

void usage()
{
  static	char	*usagestr[] =  {
    "chotrec (Ver 0.06) (c)1997 ken-ichi HAYASHI\n",
    "qvrec [options] filename1.jpg filename2.jpg ...\n",
    "\t -h           : show this usage.\n",
    "\t -S speed     : serial speed. [normal middle high]\n",
#ifndef X68
    "\t -D ttydevice : set tty(cua) device.\n",
#endif
    (char *)NULL,
  };
  char	**p;

  p = usagestr;
  while (*p)
    fprintf(stderr, *p++);
}

void Exit(code)
     int code;
{
  if (!(F1getfd() < 0))
    F1reset();
    closetty(F1getfd());
  exit(code);
} 

void
put_jpeg(infilename)
     char *infilename;
{
  int	len;
  FILE	*infp;
  int i;
  int j;
  int k;
  int m;
  u_short s;
  u_short id;
  u_int length = 0;
  int skip = 0;
  u_char *u;
  u_char c;
  u_char magic[2] = {0xff, 0xd8};
  struct stat st;
  u_char buf[0x400];

  infp = stdin;
  if (infilename) {
    infp = fopen(infilename, RMODE);
    if (infp == NULL){
      fprintf(stderr, "can't open infile(%s).\n", infilename);
      errflg ++;
      goto cleanup;
    }
  }

/*
  if(fread(lbuf, sizeof(u_char), 2, infp) < 0 ){
    fprintf(stderr, "can't read header.\n");
    errflg ++;
    goto cleanup;
  }
  if(memcmp(lbuf, magic, sizeof(magic)) != 0){
    fprintf(stderr, "this file(%s) is not JPEG format.\n", infilename);
    errflg ++;
    goto cleanup;
  }
*/

  if(stat(infilename, &st) == 0){
    length = (int) st.st_size;
    fprintf(stderr, "%s:size = %d\n", infilename, length);
  }


  i = length;
  while(i > 0){
    j = fread(buf, sizeof(u_char), 0x400, infp);
    if(j <0){
      fprintf(stderr, "fread err\n");
      errflg ++;
      goto cleanup;
    }
    m = 0;
    for(k = 0 ; k < j; k++){
      c = buf[k];
      if((c == 0x7D) || (c == 0xC1) || (c == 0xC0))
	m++;
      m++;
    }
    if(i == length)
      F1fwrite(buf, m, 0);
    else if(i < 0x400){
      F1fwrite(buf, m, 2);
      goto cleanup;
    }else
      F1fwrite(buf, m, 1);
    i = i - j;
    fprintf(stderr,"%d\n",i);
  }

/*  if (len < 0) {
    errflg ++;
  }
*/

 cleanup:;
  if (infp != stdin)
    fclose(infp);

  return;
}


#ifndef DONTCAREUID
void
daemonuid()
{
  if (uidswapped) {
#ifdef HAVE_SETREUID
    setreuid(uid, euid);
    setregid(gid, egid);
#else
    setuid(uid);
    seteuid(euid);
    setgid(gid);
    setegid(egid);
#endif
    uidswapped = 0;
  }
}

void
useruid()
{
  if (!uidswapped) {
#ifdef HAVE_SETREUID
    setregid(egid, gid);
    setreuid(euid, uid);
#else
    setgid(egid);
    setegid(gid);
    setuid(euid);
    seteuid(uid);
#endif
    uidswapped = 1;
  }
}
#endif

void
main(argc, argv)
     int	argc;
     char	**argv;
{
  char	*devpath = NULL;
  char	*outfilename = NULL;
  char	c;

#ifndef DONTCAREUID
  uid = getuid();
  euid = geteuid();
  gid = getgid();
  egid = getegid();
  useruid();
#endif

  devpath = getenv("CHOTPLAYTTY");

  while ((c = getopt( argc, argv, "D:S:F:hv")) != -1){
    switch(c) {
    case 'h':
    case '?':
      usage();
      exit(-1);
    case 'v':
      verbose = 1;
      break;
    case 'D':
      devpath = optarg;
      break;
    case 'S':
      switch(optarg[0]){
      case 'h':
      case '3':
	speed = B38400;
	break;
      case 'm':
      case '2':
	speed = B19200;
	break;
      case 'n':
      case '1':
	speed = B9600;
	break;
      default:
	speed = B38400;
	break;
      }
      break;
    case 'F':
      switch(optarg[0]){
      case 'j':
      case 'J':
	format = JPEG;
	break;
      case 'p':
      case 'P':
	format = PMP;
	break;
      default:
	format = JPEG;
      }
      break;
    }
  }

  if(devpath == NULL){
    devpath = malloc(sizeof(char) * (strlen(RSPORT) +1));
    if(devpath == NULL) {
      fprintf(stderr, "can't malloc\n");
      exit(1);
    }
    strcpy(devpath, RSPORT);
  }

  if(devpath){
#ifndef DONTCAREUID
	  daemonuid();
#endif
	  F1setfd(opentty(devpath));
#ifndef DONTCAREUID
	  useruid();
#endif
	  if (F1getfd() < 0)
		  Exit(1);
  }
  changespeed(F1getfd(), speed);
 
  F1ok();
/*  F1status(1);
  F1ffs();
  Exit(0);
*/
  F1ffs();
  if(optind == argc){
#ifdef BINARYFILEMODE
#ifdef WIN32
  	_setmode(_fileno(stdin), _O_BINARY);
#else
  	setmode(fileno(stdin), O_BINARY);
#endif
#endif
	switch(format){
	case JPEG:
	  put_jpeg(all_pic_num + 1, NULL);
	  break;
	default:
	  put_jpeg(NULL);
	  break;
	}
  }
  if(optind < argc){
    while(optind < argc){
/*      all_pic_num = QVhowmany(); */
/*      if (all_pic_num < 0) Exit(1); */
      switch(format){
      case JPEG:
	put_jpeg(argv[optind++]);
	break;
      default:
	put_jpeg(argv[optind++]);
	break;
      }
    }
  }

  Exit (errflg ? 1 : 0);
}
