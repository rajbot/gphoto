#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "defs.h"
#include "transmission.h"
#include "lowlevel.h"
#include "getPicInfo.h"
#include "error.h"
#include "open.h"

int qm100_transmit(int serialdev, unsigned char *cmd, int cmd_len, qm100_packet_block *packet)
{

  qm100_attention(serialdev);
  qm100_sendPacket(serialdev, cmd, cmd_len);
  qm100_getAck(serialdev);
  qm100_getPacket(serialdev, packet);
  qm100_endTransmit(serialdev);

  return (1);
}

void qm100_attention(int serialdev)
{
  char c;
  do {
    qm100_writeByte(serialdev, 0x05);
    c=qm100_readTimedByte(serialdev);
  } while (c == 0);
  c=qm100_readByte(serialdev);

  if (c != 0x06)
    {
      qm100_error(serialdev, "Camera is not online");
    }
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
  packet[packet_pos]=0x02;
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
  packet[packet_pos]=0x03;
  packet_pos++;
  packet_sum += 0x03;
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
  
#ifdef _CLI_
  if (qm100_showWritePackages) qm100_iostat("send :", packet, packet_pos);
#endif

  if ((write(serialdev, packet, packet_pos)) < packet_pos) {
    qm100_error(serialdev, "Cannot write to device");
  }
}

void qm100_getAck(int serialdev)
{
  char c;
  
  c=qm100_readByte(serialdev);
  if (c != 0x06) qm100_error(serialdev, "Acknowledgement Failed");
  qm100_writeByte(serialdev, 0x04);

  c=qm100_readByte(serialdev);

  if (c != 0x05) qm100_error(serialdev, "Acknowledgement Failed");
  qm100_writeByte(serialdev, 0x06);
}


int qm100_getPacket(int serialdev, qm100_packet_block *packet)
{
  unsigned char c, qm100_sum=0, sum=0;
  short len, pos=0;
  
  c = qm100_readByte(serialdev);
  if (c != 0x02) qm100_error(serialdev, "Get package failed");
  
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
  if (c==0x03)
    {
      packet->transmission_continues=0;
    }
  else if (c==0x17)
    {
      packet->transmission_continues=1;
    }
  else qm100_error(serialdev, "qm100: failed trans cont status");
  sum+=c;
  sum=(sum & 0xff);
  qm100_sum=qm100_readCodedByte(serialdev);

#ifdef _CLI_
  if (qm100_showReadPackages) qm100_iostat("recv :", packet->packet, packet->packet_len);
#endif
  
  if (qm100_sum != sum) 
    {
      printf("Checksum wrong : read (0x%x) : calc (0x%x)\n",qm100_sum, sum);
    }
  return 1;
}

void qm100_endTransmit(int serialdev)
{
  char c;
  qm100_writeByte(serialdev, 0x06);
  c=qm100_readByte(serialdev);
  if (c != 0x04) qm100_error(serialdev, "End of Transmission Failed");
}

void qm100_continueTransmission(int serialdev)
{
  char c;
  qm100_writeByte(serialdev, 0x06);
  c=qm100_readByte(serialdev);
  if (c != 0x04) qm100_error(serialdev, "qm100_continueTransmission expected 4");
  c=qm100_readByte(serialdev);
  if (c != 0x05) qm100_error(serialdev, "qm100_continueTransmission expected 5");
  qm100_writeByte(serialdev, 0x06);
}

int qm100_getRealPicNum(int serialdev, int picNum)
{
  qm100_packet_block packet;
  char qm100_filename[6];
  int realPicNum;

  qm100_getPicInfo(serialdev, picNum, &packet);
  memcpy(&qm100_filename, &packet.packet[183], 5);
  sscanf(qm100_filename,"%d",&realPicNum);
  return (realPicNum);
}







