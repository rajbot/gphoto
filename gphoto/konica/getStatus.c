#include <string.h>
#include <stdio.h>

#include "defs.h"
#include "getStatus.h"
#include "transmission.h"
#include "error.h"

void qm100_getStatus(int serialdev)
{
  qm100_packet_block packet;
  char cmd_status[QM100_STATUS_LEN]=QM100_STATUS;

  memset(&packet, 0, sizeof(packet));
  packet = qm100_transmit(serialdev, cmd_status, sizeof(cmd_status));
  if (packet.packet_len == 34)
    {

#ifdef _CLI_
      if (qm100_showStatus)
        {
	  unsigned char counter=0;
	  qm100_showStatus=0;
          printf("Konica QM100 Status :\n");
          printf(" Date     : %u:%u:%u %u/%u/%u\n",TIME_HOUR,TIME_MIN,TIME_SEC,TIME_DAY,TIME_MON,TIME_YEAR);
	  counter = packet.packet[30];
          printf(" Pictures : %d\n",PICTURE_COUNT);
          printf(" Counter  : %d\n",counter);
          printf(" Flash    : ");
          {
            if (FLASH == FLASH_OFF) printf("Off");
            if (FLASH & FLASH_ON) printf("On ");
            if (FLASH & FLASH_AUTO) printf("Auto ");
            if (FLASH & FLASH_REDEYE) printf("Red");
            printf("\n");
          }
          printf(" Quality  : ");
          {
            if (QUALITY == QUALITY_SUPER) printf ("Super Fine\n");
            if (QUALITY == QUALITY_FINE) printf ("Fine\n");
            if (QUALITY == QUALITY_ECONOMY) printf ("Economy\n");
          }
          printf(" Focus    : ");
          {
            if (FOCUS == FOCUS_AUTO) printf("Auto\n");
            if (FOCUS == FOCUS_MANUAL) printf("Manual\n");
            if (FOCUS == FOCUS_MACRO) printf("Macro\n");
          }
          printf(" Exposure : %d.0\n",(int)EXPOSURE);
          printf(" WhiteBalance : ");
          {
	    switch (WHITEBAL)
	      {
	      case 0x0e: printf("Auto"); break;
	      case 0x10: printf("Indoors"); break;
	      default: printf("Unknown"); break;
	      }
            printf("\n");
          }
        }
#endif
    }
  else qm100_error(serialdev, "Len wrong");
}
