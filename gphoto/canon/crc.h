#ifndef CRC_H
#define CRC_H

#include <stdint.h>

uint16_t canon_psa50_gen_crc(const unsigned char *pkt,int len);
int canon_psa50_chk_crc(const unsigned char *pkt,int len,uint16_t crc);

#endif
