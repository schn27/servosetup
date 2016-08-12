#ifndef CRC_H
#define CRC_H

#include <stdint.h>

uint8_t Crc8(uint8_t *pcBlock, int len);
uint16_t Crc16(uint8_t *pcBlock, int len);

uint8_t CheckSum(uint8_t *data, int len);

#endif
