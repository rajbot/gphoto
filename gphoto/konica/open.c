#include "qm100.h"

/*---------------------------------------------------------------------*
 *                                                                     *
 * getKeyword - get value for a keyword/variable, from                 *
 *              the first successful test:                             *
 *                                                                     *
 *            1.  Look for an environment variable of the              *
 *                form "QM100_varname"                                 *
 *            2.  Look in ~/.gphoto/konicarc for a line                *
 *                beginning with varname                               *
 *            3.  default value provided by caller.                    *
 *                                                                     *
 *---------------------------------------------------------------------*/
char *qm100_getKeyword(char *key, char *dflt)
{
   char         fname[128];
   FILE        *fp;
   char        *sp=NULL;
   static char  buf[256];
   
   sprintf(buf, "QM100_%s", key);
   sp = getenv(buf);
   if (!sp)
      {
      sprintf(fname, "%s/.gphoto/konicarc", getenv("HOME"));
      fp = fopen(fname, "r");
      if (!fp)
         {
         sprintf(fname, "%s/konicarc", getenv("HOME"));
         fp = fopen(fname, "r");
      }
      if (fp)
      {
      while ((sp = fgets(buf, sizeof(buf)-1, fp)) != NULL)
         {
         if (*sp == '#' || *sp == '*')
            continue;
         sp = strtok(buf, " \t\r\n");
         if (!sp)       
            continue;    /* skip blank lines */
         if (strcasecmp(sp, key) != 0)
            continue;
         sp = strtok(NULL, " \t\r\n");
         break;
         }
      fclose(fp);
      }
      else
         printf("Unable to open %s\n", fname);
      }
   if (!sp)
      sp = dflt;
   return sp;
}

/*---------------------------------------------------------------------*
 *                                                                     *
 * setTrace - set tracing options from environment and/or              *
 *            ~/.gphoto/konicarc:                                      *
 *                                                                     *
 *            Trace - specifies name of file, or "Off".                *
 *                    "On" is synonym for ~/.gphoto/konica.trace       *
 *            Trace_Bytes - 'On' indicates that low-level byte         *
 *                    trace should be included.                        *
 *                                                                     *
 *---------------------------------------------------------------------*/
void qm100_setTrace(void)
{
   char *fname;
   char tname[128];
   
   fname = qm100_getKeyword("TRACE", "off");
   if (!qm100_trace && fname &&
       strcasecmp(fname, "off") != 0  &&
       strcasecmp(fname, "none") != 0)
      {
      if (strcasecmp(fname, "on") == 0)
         fname = "konica.trace";
      if (*fname != '/' && *fname != '.')
         sprintf(tname, "%s/.gphoto/%s", getenv("HOME"),  fname);
      else
         strcpy(tname, fname);
      qm100_trace = fopen(tname, "w");
      if (!qm100_trace)
         {
         sprintf(tname, "%s/%s", fname);
         qm100_trace = fopen(tname, "w");
         }
      }
   
   fname = qm100_getKeyword("TRACE_BYTES", "off");
   if (qm100_trace && fname && strcasecmp(fname, "off") != 0)
      qm100_showBytes = 1;
}

/*---------------------------------------------------------------------*
 *                                                                     *
 * open  - prepare serial port for use, and send initialization        *
 *         packets to camera.                                          *
 *                                                                     *
 *---------------------------------------------------------------------*/
int qm100_open(const char *devname)
{
  int serialdev;
  qm100_packet_block packet;
  char cmd[]=QM100_INIT;

  serialdev = open(devname, O_RDWR|O_NOCTTY);
  if (serialdev < 0) 
     qm100_error(serialdev, "Unable to open serial device", errno);
  
  if (tcgetattr(serialdev, &oldt) < 0) 
     qm100_error(serialdev, "Unable to get serial device attributes", errno);

  memcpy((char *)&newt,(char *)&oldt, sizeof(struct termios));
  newt.c_cflag |= CS8 | HUPCL;
  newt.c_iflag &= ~(IGNBRK|BRKINT|IGNPAR|PARMRK|INPCK|ISTRIP|INLCR);
  newt.c_iflag &= ~(IGNCR|ICRNL|IXON|IXOFF|IXANY|IMAXBEL);
  
  newt.c_oflag &= ~(OPOST);
  newt.c_lflag &= ~(ISIG|ICANON);
  newt.c_cc[VMIN] = 1;
  newt.c_cc[VTIME] = 0;

  cfsetospeed(&newt, B9600);
  cfsetispeed(&newt, B9600);

  if (tcsetattr(serialdev, TCSANOW, &newt) < 0)
     qm100_error(serialdev, "Unable to set serial device attributes", errno);
  qm100_transmit(serialdev, cmd, sizeof(cmd), &packet, "Open");
  qm100_setSpeed(serialdev, qm100_transmitSpeed);
  return serialdev;
}






