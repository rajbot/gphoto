#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#include "qm100.h"
#include "defs.h"
#include "getPicInfo.h"
#include "transmission.h"
#include "open.h"
#include "close.h"
#include "setSpeed.h"
#include "formatCF.h"
#include "savePic.h"
#include "saveThumb.h"
#include "erasePic.h"
#include "getStatus.h"

struct termios newt, oldt;

void main(int argc, char *argv[])
{
  int c;
  int serialdev;
  while (1)
    {
      c = getopt(argc, argv, "e:fg:hr:st:w:");
      if (c == -1)
	break;
 
      switch (c)
	{
	case 's':
	  qm100_showStatus=1;
	  qm100_permission=1;
	  break;
	 
	case 'e':
	  sscanf(optarg,"%d",&qm100_killPic);
	  qm100_permission=1;
	  break;
	  
	case 'f':
	  qm100_format=1;
	  qm100_permission=1;
	  break;
	  
	case 'g':
	  sscanf(optarg,"%d",&qm100_getPic);
	  qm100_permission=1;
	  break;
	  
	case 't':
	  sscanf(optarg,"%d",&qm100_getThumb);
	  qm100_permission=1;
	  break;
	  
	case 'w':
	  switch ((char)*optarg)
	    {
	    case 'b':
	      qm100_showWriteBytes=1;
	      break;
	    case 'p':
	      qm100_showWritePackages=1;
	      break;
	    default:
	      printf("qm100: either b or p must follow -w option\n");
	    }
          break;

	case 'r':
          switch ((char)*optarg)
            {
	    case 'b':
              qm100_showReadBytes=1;
	      break;
	    case 'p':
              qm100_showReadPackages=1;
	      break;
            default:
              printf("either b or p must follow -r option\n");
	    }
          break;
	default:
	  printf("Konica QM-100 Digtal Camera Utility\n");
	  printf("Usage: qm100 MODE [OPTION]... [FILENAME]\n");
	  printf("\tMODES\n");
	  printf("\t  -s\t\tdisplay the camera status\n");
	  printf("\t  -g##\t\tget picture ## (needs filename)\n");
	  printf("\t  -t##\t\tget thumbnail ## (needs filename)\n");
	  printf("\t  -e##\t\terase picture ##\n");
	  printf("\t  -c\t\ttake picture\n");
	  printf("\t  -f\t\tformat the compact flash card\n");
	  printf("\tOPTIONS\n");
	  printf("\t  -r[b|p]\tdisplay read bytes/packets\n");
	  printf("\t  -w[b|p]\tdisplay written bytes/packets\n");
	  printf("\t  -h\t\tdisplay this help\n");
	  printf("\n\tplease supply ONE major mode (-s, -g, -t, -f)\n");
	  exit(0);
	}
    }
  
  if (qm100_permission)
    {
      serialdev = qm100_open(DEFAULT_PORT);
      qm100_getStatus(serialdev);

      if (qm100_format)
	{
	  qm100_formatCF(serialdev);
	}
      else
	{
	  qm100_setSpeed(serialdev, B115200);

	  if (qm100_getPic)
	    {
	      qm100_getPic = qm100_getRealPicNum(serialdev, qm100_getPic);
	      qm100_savePic(serialdev, argv[optind], qm100_getPic);
	    }
	  if (qm100_killPic)
	    {
	      qm100_killPic = qm100_getRealPicNum(serialdev, qm100_killPic);
	      qm100_erasePic(serialdev, qm100_killPic);
	    }
	  if (qm100_getThumb)
	    {
	      qm100_getThumb = qm100_getRealPicNum(serialdev, qm100_getThumb);
	      qm100_saveThumb(serialdev, argv[optind], qm100_getThumb);
	    }
	  qm100_setSpeed(serialdev, B9600);
	}	 
      qm100_close(serialdev);
    }
  else
    {
      printf("qm100: no MODE specified - try \"qm100 -h\"\n");
    }
}








































