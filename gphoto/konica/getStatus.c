#include "qm100.h"
#include <time.h>

char *qm100_getDate(int serialdev)
{
   static char fmtdate[64];
   qm100_packet_block packet;
   char cmd[] = QM100_GETTIME;
   PKT_TIME *pt = (PKT_TIME *) &(packet.packet);
  
   qm100_transmit(serialdev, cmd, sizeof(cmd), &packet, "GetTime");
   if (pt->year <60)
      pt->year += 100;
   sprintf(fmtdate, "%4.4d %2.2d/%2.2d %2.2d:%2.2d:%2.2d",
           pt->year+1900, pt->month, pt->day, pt->hour,
           pt->minute, pt->second);
   return fmtdate;
}

char *qm100_setDate(int serialdev)
{
   qm100_packet_block packet;
   struct tm *tp;
   time_t    mytime;
   char cmd[] = QM100_SETTIME;
   
   mytime = time(NULL);
   tp = localtime(&mytime);
   if (tp->tm_year > 99)
      cmd[4] = tp->tm_year - 100;
   else
      cmd[4] = tp->tm_year;
   cmd[5] = tp->tm_mon;
   cmd[6] = tp->tm_mday;
   cmd[7] = tp->tm_hour;
   cmd[8] = tp->tm_min;
   cmd[9] = tp->tm_sec;
   qm100_transmit(serialdev, cmd, sizeof(cmd), &packet, "SetTime");
   return qm100_getDate(serialdev);
}
   
void qm100_getStatus(int serialdev, int showStatus)
{
   qm100_packet_block packet;
   
   char cmd[]=QM100_GETSTATUS;
   
   memset(&packet, 0, sizeof(packet));
   qm100_transmit(serialdev, cmd, sizeof(cmd), &packet, "GetStatus");
   if (packet.packet_len == 34)
      {
      qm100_pictureCount = PICTURE_COUNT;
      if (showStatus)
         {
         printf("Konica QM100 Status :\n");
         printf(" Date     : %u:%u:%u %u/%u/%u\n",
                TIME_HOUR, TIME_MIN, TIME_SEC, TIME_DAY, TIME_MON, TIME_YEAR);
         printf(" Pictures : %d\n",PICTURE_COUNT);
         printf(" Counter  : %u\n",COUNTER);
         printf(" Flash    : ");
         if (FLASH == FLASH_OFF) printf("Off");
         if (FLASH & FLASH_ON) printf("On ");
         if (FLASH & FLASH_AUTO) printf("Auto ");
         if (FLASH & FLASH_REDEYE) printf("Red");
         printf("\n");
         
         printf(" Quality  : ");
         if (QUALITY == QUALITY_SUPER) printf ("Super Fine\n");
         if (QUALITY == QUALITY_FINE) printf ("Fine\n");
         if (QUALITY == QUALITY_ECONOMY) printf ("Economy\n");
         printf(" Focus    : ");
         if (FOCUS == FOCUS_AUTO) printf("Auto\n");
         if (FOCUS == FOCUS_MANUAL) printf("Manual\n");
         if (FOCUS == FOCUS_MACRO) printf("Macro\n");
         printf(" Exposure : %d.0\n",(int)EXPOSURE);
         printf(" WhiteBalance : ");
         switch (WHITEBAL)
            {
            case 0x0e: printf("Auto"); break;
            case 0x10: printf("Indoors"); break;
            default: printf("Unknown"); break;
            }
         printf("\n");
         }
      }
   else qm100_error(serialdev, "GetStatus incorrect response length", 0);
}
