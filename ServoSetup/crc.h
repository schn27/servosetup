#ifndef CRC_H
#define CRC_H

#include <stdint.h>

namespace crc {

uint8_t crc8(const uint8_t *data, size_t len);
uint16_t crc16(const uint8_t *data, size_t len);
uint8_t checkSum(const uint8_t *data, size_t len);

}

#endif
