#define DEFINE_GLOBALS
#include "qm100.h"

int qm100_transmit(int serialdev, unsigned char *cmd, int cmd_len, 
                   qm100_packet_block *packet, char *title)
{
   
   if (qm100_trace) 
      fprintf(qm100_trace, "Beginning operation: %s\n", title);
   qm100_attention(serialdev);
   qm100_sendPacket(serialdev, cmd, cmd_len);
   qm100_getAck(serialdev);
   qm100_getPacket(serialdev, packet);
   if (!packet->transmission_continues)
      qm100_endTransmit(serialdev, title);
   return (1);
}

void qm100_attention(int serialdev)
{
   char c;
   int  limit = 100;  
   
   do 
      {
      /*---------------------------------------------------------------*
       *                                                               *
       * Each time through this loop, we try to determine if a byte is *
       * available from the camera.  If no byte is available within    *
       * the timeout interval of 10000 micro-secs, (10ms), we simply   *
       * try again.  To prevent a permanent hang in case the camera    *
       * is not working at all, we limit ourselves to 100 attempts,    *
       * which is a total elapsed time of about 10 seconds.            *
       *                                                               *
       *---------------------------------------------------------------*/
      qm100_writeByte(serialdev, SIO_ENQ);
      c=qm100_readTimedByte(serialdev);
      } while (c == 0 && --limit >0);
   if (c)
      c=qm100_readByte(serialdev);
   if (c != SIO_ACK)
      {
      if (qm100_trace)
         fprintf(qm100_trace, "Attention response invalid - %02x\n", c);
      qm100_error(serialdev, "Camera is not online", 0);
      }
   if (qm100_trace)
      fprintf(qm100_trace, "Attention acknowledged by camera\n");
}

void qm100_sendPacket(int serialdev, unsigned char *cmd, int cmd_len)
{
   unsigned char esc_list[255];
   unsigned char packet[255];
   unsigned char packet_sum=0;
   int packet_pos=0, cmd_pos=0, esc_lookup=0;
   int data;
   
   memset(&esc_list, 0, sizeof(esc_list));
   esc_list[0x02] = 0xfd;
   esc_list[0x03] = 0xfc;
   esc_list[0x05] = 0xfa;
   esc_list[0x06] = 0xf9;
   esc_list[0x11] = 0xee;
   esc_list[0x1b] = 0xe4;
   
   memset(&packet, 0, sizeof(packet));
   packet[packet_pos]=SIO_STX;
   packet_pos++;
   
   esc_lookup = esc_list[ (cmd_len & 0xff) ];
   packet_sum+=(cmd_len & 0xff);
   if (esc_lookup)
      {
      packet[packet_pos]=0x1b;
      packet_pos++;
      packet[packet_pos]=esc_lookup;
      packet_pos++;
      }
   else
      {
      packet[packet_pos]=(cmd_len & 0xff);
      packet_pos++;
      }
   
   esc_lookup = esc_list[((cmd_len>>8) & 0xff)];
   packet_sum+=((cmd_len>>8) & 0xff);
   if (esc_lookup)
      {
      packet[packet_pos]=0x1b;
      packet_pos++;
      packet[packet_pos]=esc_lookup;
      packet_pos++;
      }
   else
      {
      packet[packet_pos]=((cmd_len>>8) & 0xff);
      packet_pos++;
      }
   
   while (cmd_pos < cmd_len)
      {
      data=cmd[cmd_pos];
      cmd_pos++;
      packet_sum+=data;
      
      esc_lookup = esc_list[data];
      if (esc_lookup)
         {
         packet[packet_pos]=0x1b;
         packet_pos++;
         packet[packet_pos]=esc_lookup;
         packet_pos++;
         }
      else
         {
         packet[packet_pos]=data;
         packet_pos++;
         }
      
      }
   packet[packet_pos]=SIO_ETX;
   packet_pos++;
   packet_sum += SIO_ETX;
   packet_sum = packet_sum & 0xff;
   
   esc_lookup = esc_list[packet_sum];
   if (esc_lookup)
      {
      packet[packet_pos]=0x1b;
      packet_pos++;
      packet[packet_pos]=esc_lookup;
      packet_pos++;
      }
   else
      {
      packet[packet_pos]=packet_sum;
      packet_pos++;
      }
   
   if (qm100_trace) 
      dump(qm100_trace, "Send Packet", packet, packet_pos);
   
   if ((write(serialdev, packet, packet_pos)) < packet_pos) 
      qm100_error(serialdev, "Cannot write to device", errno);
}

void qm100_getAck(int serialdev)
{
   char c;
   
   c=qm100_readByte(serialdev);
   if (c != SIO_ACK)
      qm100_error(serialdev, "Acknowledgement Failed", 0);
   qm100_writeByte(serialdev, SIO_EOT);
   
   c=qm100_readByte(serialdev);
   
   if (c != SIO_ENQ) 
      qm100_error(serialdev, "Acknowledgement Failed", 0);
   qm100_writeByte(serialdev, SIO_ACK);
}


int qm100_getPacket(int serialdev, qm100_packet_block *packet)
{
   unsigned char c, qm100_sum=0, sum=0;
   short len, pos=0;
   char *perror = NULL;
   static retries=0;

   ++retries;
   c = qm100_readByte(serialdev);
   if (c != SIO_STX) 
      qm100_error(serialdev, "Invalid response header byte", 0);
   c=qm100_readCodedByte(serialdev);
   len=c;
   sum=c;
   c=qm100_readCodedByte(serialdev);
   len += (c<<8);
   sum += c;
   packet->packet_len=len;
   while (len--)
      {
      c=qm100_readCodedByte(serialdev);
      packet->packet[pos]=c;
      pos++;
      sum+=c;
      }
   c=qm100_readByte(serialdev);
   if (c==SIO_ETX)
      packet->transmission_continues=0;
   else if (c==SIO_ETB)
      packet->transmission_continues=1;
   else
      perror = "Packet error: Unexpected trailer byte";
   sum+=c;
   sum=(sum & 0xff);
   qm100_sum=qm100_readCodedByte(serialdev);
   
   if (qm100_trace) 
      dump(qm100_trace, "Receive Packet", packet->packet, packet->packet_len);
   
   if (qm100_sum != sum) 
      {
      printf("Incorrect checksum: read (0x%x) : calc (0x%x)\n",qm100_sum, sum);
      if (retries <= 5)
         {
         printf("Attempting retry - retry depth=%d\n", retries);
         qm100_writeByte(serialdev, SIO_NAK);
         qm100_getPacket(serialdev, packet);
         }
      else
         perror = "Packet error: incorrect checksum";
      }
   --retries;
   if (perror)
      qm100_error(serialdev, perror, 0);
   return 0;
}

void qm100_endTransmit(int serialdev, char *title)
{
   char c;
   qm100_writeByte(serialdev, SIO_ACK);
   c=qm100_readByte(serialdev);
   if (c != SIO_EOT) 
      qm100_error(serialdev, "End of Transmission Failed", 0);
   if (qm100_trace)
      fprintf(qm100_trace, "End transmission for %s\n", title);
}

void qm100_continueTransmission(int serialdev, char *title)
{
   char c;
   
   if (qm100_trace)
      fprintf(qm100_trace, "Continue transmission for %s\n", title);
   qm100_writeByte(serialdev, SIO_ACK);
   c=qm100_readByte(serialdev);
   if (c != SIO_EOT) 
      qm100_error(serialdev, "Unexpected response to continue Transmission", 0);
   c=qm100_readByte(serialdev);
   if (c != SIO_ENQ) 
      qm100_error(serialdev, "Unexpected response to continue Transmission", 0);
   qm100_writeByte(serialdev, SIO_ACK);
}

int qm100_getRealPicNum(int serialdev, int picNum)
{
   qm100_packet_block packet;
   int realPicNum;
   
   qm100_getPicInfo(serialdev, picNum, &packet);
   if (packet.packet_len == 0x3ff)
      sscanf(PICNUM, "%d", &realPicNum);       // hpc20
   else if (packet.packet_len == 0x37a)
      sscanf(PICNUM-0x42, "%d", &realPicNum);  // hpc30
   else
      qm100_error(serialdev,"Unexpected packet length in response to getPicInfo",0);
   return (realPicNum);
}

/*---------------------------------------------------------------------*
 *                                                                     *
 * qm100_getCommandTermination                                                   *
 *                                                                     *
 *---------------------------------------------------------------------*/
void  qm100_getCommandTermination(int serialdev)
{
   qm100_packet_block packet;
   char c;
      
   c = qm100_readByte(serialdev);
   if (c != SIO_ENQ)
      qm100_error(serialdev,
                  "Unexpected data while expecting command termination packet", 0);
   qm100_writeByte(serialdev, SIO_ACK);
   qm100_getPacket(serialdev, &packet);
}

void  qm100_setTransmitSpeed(void)
{
   char *sp;
   
   sp = qm100_getKeyword("SPEED", DEFAULT_SPEED);
   while (qm100_transmitSpeed == 0)
      {
      int l;
      
      if (!sp)
         sp = DEFAULT_SPEED;
      l = strlen(sp);
      if (strncmp(sp, "115200", l) == 0)
         qm100_transmitSpeed = B115200;
      else if (strncmp(sp, "57600", l) == 0)
         qm100_transmitSpeed = B57600;
      else if (strncmp(sp, "38400", l) == 0)
         qm100_transmitSpeed = B38400;
      else if (strncmp(sp, "19200", l) == 0)
         qm100_transmitSpeed = B19200;
      else if (strncmp(sp, "9600", l) == 0)
         qm100_transmitSpeed = B9600;
      else
         {
         printf("qm100:  Invalid speed %s - using default (%s)\n",
                sp, DEFAULT_SPEED);
         sp = NULL;
         }
      }

   sp = qm100_getKeyword("PACING", DEFAULT_PACING);
   while (qm100_sendPacing == 0)
      {
      qm100_sendPacing = atoi(sp);
      if (qm100_sendPacing < 1)
         {
         printf("qm100:  Invalid pacing value %s - using default (%s)\n",
                sp, DEFAULT_PACING);
         sp = DEFAULT_PACING;
         }
      }
}
