#include "qm100.h"

static char cycle[] = "|/-\\";
void progress(void)
{
   static int dots = 0;
   
   printf("\b%c", cycle[dots]);
   fflush(stdout);
   if (++dots >= sizeof(cycle)-1)
      dots=0;
}

int main(int argc, char *argv[])
{
   int   c;
   int   serialdev;
   int   infoPic=0;
   char  cameraName[128];
   int   qm100_date=0;

   strcpy(cameraName,qm100_getKeyword("CAMERA", DEFAULT_PORT));
   qm100_setTrace();
   while (1)
      {
      c = getopt(argc, argv, "e:fg:hstai:p:b:dD");
      if (c == -1)
         break;
      switch (c)
         {
         case 'b':
            {
            char tval[64];
            sprintf(tval, "QM100_SPEED=%s", optarg);
            putenv(tval);
            break;
            }
         case 'p':
            strcpy(cameraName, optarg);
            break;
            
         case 'i':
            qm100_permission=1;
            sscanf(optarg,"%d",&infoPic);
            break;

         case 'a':
            qm100_permission=1;
            qm100_getPic = -1;
            break;
            
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

         case 'd':
            qm100_date=1;
            qm100_permission=1;
            break;
	  
         case 'D':
            qm100_date=2;
            qm100_permission=1;
            break;
	  
         case 'g':
            sscanf(optarg,"%d",&qm100_getPic);
            qm100_permission=1;
            break;
	  
         case 't':
            qm100_getThumb = 1;
            qm100_permission=1;
            break;

         default:
            printf("Konica QM-100 Digtal Camera Utility "
                   "Version %s Mod %s\n", QM100_VER,  QM100_MOD);
            printf("Usage:\tqm100 MODE [OPTION]... [FILENAME]\n");
            printf("\tMODES\n");
            printf("\t  -s\t\tdisplay the camera status\n");
            printf("\t  -a\t\tget all pictures\n");
            printf("\t  -t\t\tget all thumbnails\n");
            printf("\t  -g##\t\tget picture ## (needs filename)\n");
            printf("\t  -e##\t\terase picture ##\n");
            printf("\t  -i##\t\tdump information block for picture ##\n");
            printf("\t  -f\t\tformat the compact flash card\n");
            printf("\t  -d\t\tget date and time from camera\n");
            printf("\t  -D\t\tSet camera date and time from system clock\n");
            printf("\n\tOPTIONS\n");
            printf("\t  -p\t\tSet the camera device/port (default %s)\n", cameraName);
            printf("\t  -b\t\tSet the speed/baud rate\n"
                   "\t    \t\tChoose from 9600, 19200, 38400, 57600, or 115200\n"
                   "\t    \t\t(Default %s)\n", DEFAULT_SPEED);
            printf("\t  -h\t\tdisplay this help\n");
            printf("\n\tplease supply ONE major mode (-s, -g, -t, -f, -e, -a)\n");
            printf("\n");
            printf("\tEnvironment variables:\n");
            printf("\t  QM100_CAMERA=dev  - set default device for camera\n");
            printf("\t  QM100_SPEED=nn    - set transmission speed\n");
            printf("\t  QM100_PACING=nn   - set I/O pacing delay, in millesconds\n");
            printf("\t                      (Default %d)\n", DEFAULT_PACING);
            printf("\t  QM100_TRACE=fname - trace camera activity to fname\n");
            printf("\t  QM100_TRACE_BYTES - trace low-level camera I/O\n");
            exit(0);
         }
      }

   qm100_setTransmitSpeed();
   if (qm100_permission)
      {
      serialdev = qm100_open(cameraName);
      qm100_getStatus(serialdev, qm100_showStatus);
      if (qm100_format)
         qm100_formatCF(serialdev);
      else if (qm100_date == 1)
         printf("Camera reports date as: %s\n", qm100_getDate(serialdev));
      else if (qm100_date == 2)
         printf("Camera date set to: %s\n", qm100_setDate(serialdev));
      else if (qm100_getPic > 0)
         {
         qm100_getPic = qm100_getRealPicNum(serialdev, qm100_getPic);
         printf("Retrieving %s  ", argv[optind]);
         qm100_savePic(serialdev, argv[optind], qm100_getPic, progress);
         printf("\b - Done.\n");
         }
      else if (infoPic > 0)
         {
         qm100_packet_block packet;
         qm100_getPicInfo(serialdev, infoPic, &packet);
         dump(stdout, "PicInfo", packet.packet, packet.packet_len);
         }
      else if (qm100_getPic < 0)
         {
         int i;
         char filename[40];
         
         for (i=1; i<=qm100_pictureCount; i++)
            {
            qm100_getPic = qm100_getRealPicNum(serialdev, i);
            if (qm100_getPic)
               {
               sprintf(filename, "pic%02d.jpg", qm100_getPic);
               printf("Retrieving %s  ", filename);
               fflush(stdout);
               qm100_savePic(serialdev, filename, qm100_getPic, progress);
               printf("\b - Done.\n");
               }
            }
         }
      else if (qm100_killPic)
         {
         qm100_killPic = qm100_getRealPicNum(serialdev, qm100_killPic);
         qm100_erasePic(serialdev, qm100_killPic);
         }
      else if (qm100_getThumb)
         {
         int i;
         char filename[40];
         
         for (i=1; i<=qm100_pictureCount; i++)
            {
            qm100_getThumb = qm100_getRealPicNum(serialdev, i);
            if (qm100_getThumb)
               {
               sprintf(filename, "thumb%02d.tif", qm100_getThumb);
               printf("Retrieving %s  ", filename);
               fflush(stdout);
               qm100_saveThumb(serialdev, filename, qm100_getThumb, progress);
               printf("\b - Done.\n");
               }
            }
         }
      qm100_close(serialdev);
      }
   else
      printf("qm100:  no MODE specified - try \"qm100 -h\"\n");
   return 0;
}
