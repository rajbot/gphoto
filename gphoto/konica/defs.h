#ifndef _QM100_DEFS_H
#define _QM100_DEFS_H
#define QM100_BLD      "101"
#define QM100_VER      "0.3.5"   
#define QM100_INIT     {0x00, 0x90, 0x00, 0x00}
#define QM100_ERASE    {0x00, 0x80, 0x00, 0x00, 0x02, 0x00, 0x00, 0x80}
#define QM100_FORMAT   {0x10, 0x80, 0x00, 0x00, 0x02, 0x00}
#define QM100_GETPIC   {0x30, 0x88, 0x00, 0x00, 0x02, 0x00, 0x00, 0x80}
#define QM100_GETTHUMB {0x00, 0x88, 0x00, 0x00, 0x02, 0x00, 0x00, 0x80} 
#define QM100_PICINFO  {0x20, 0x88, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00}
#define QM100_PROTECT  {0x30, 0x80, 0x00, 0x00, 0x02, 0x00, 0x00, 0x80, 0x00, 0x00} 
//                                                    MSB#  LSB#         1/0
#define QM100_SETPIC   {0xC0, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
#define QM100_SPEED    {0x80, 0x90, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00}
#define QM100_STATUS   {0x20, 0x90, 0x00, 0x00, 0x00, 0x00}
#define QM100_SUSPEND  {0x00, 0x90, 0x00, 0x00}
#define QM100_TAKEPIC  {0x00, 0x91, 0x00, 0x00, 0x02, 0x00}
#define QM100_TIME     {0xb0, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

#define QM100_INIT_LEN     4
#define QM100_STATUS_LEN   6
#define QM100_SUSPEND_LEN  4
#define QM100_SPEED_LEN    8
#define QM100_FORMAT_LEN   6
#define QM100_GETPIC_LEN   8
#define QM100_GETTHUMB_LEN 8
#define QM100_PICINFO_LEN  8
#define QM100_TIME_LEN    10
#define QM100_ERASE_LEN    8
#define QM100_SETPIC_LEN   8
#define QM100_TAKEPIC_LEN  6
#define QM100_PROTECT_LEN 10

#define FOCUS_AUTO         0
#define FOCUS_MANUAL       1
#define FOCUS_MACRO        2

#define QUALITY_SUPER      1
#define QUALITY_FINE       2
#define QUALITY_ECONOMY    3

#define FLASH_OFF          0
#define FLASH_ON           1
#define FLASH_AUTO         2
#define FLASH_REDEYE       4

#define PICTURE_COUNT (packet.packet[13]<<8|packet.packet[12])
#define TIME_SEC      packet.packet[21]
#define TIME_MIN      packet.packet[20]
#define TIME_HOUR     packet.packet[19]
#define TIME_DAY      packet.packet[18]
#define TIME_MON      packet.packet[17]
#define TIME_YEAR    (packet.packet[16]+1900)
#define FLASH         packet.packet[26]
#define QUALITY       packet.packet[27]
#define FOCUS         packet.packet[28]
#define EXPOSURE     (packet.packet[29]/10)
#define COUNTER      (packet.packet[31]<<8|packet.packet[30])
#define WHITEBAL      packet.packet[32]
#define PICNUM        packet.packet+0xf9

#define DEFAULT_PORT	  "/dev/ttyS0"
#define DEFAULT_SPEED  "38400"
#define DEFAULT_PACING "10"
#define PACKET_SIZE    4096       /* maximum response packet length */
#define STATUS_SIZE    34         /* expected size of status response* */
#define SPEED_SIZE     8          /* expected size of change speed response */

typedef struct
{
  short    packet_len;
  char     transmission_continues;
  unsigned char packet[PACKET_SIZE];
} qm100_packet_block;

typedef struct
{
  unsigned short picture_count;
  unsigned char sec;
  unsigned char min;
  unsigned char hour;
  unsigned char day;
  unsigned char mon;
  unsigned char year;
  unsigned char flash;
  unsigned char quality;
  unsigned char focus;
  unsigned char exposure;
  unsigned short counter;
  unsigned char whitebal;
} qm100_info_block;

#endif  /* _QM100_DEFS_H */
