#include "crc.h"

/*
  Name  : CRC-8
  Poly  : 0x31     x^8 + x^5 + x^4 + 1
  Init  : 0xFF
  Revert: false
  XorOut: 0x00
  Check : 0xF7 ("123456789")
  MaxLen: 15 байт(127 бит) - обнаружение
    одинарных, двойных, тройных и всех нечетных ошибок
*/
uint8_t Crc8(uint8_t *pcBlock, int len) {
	uint8_t crc = 0xFF;

	while (len--) {
		crc ^= *pcBlock++;

		for (int i = 0; i < 8; ++i) {
			crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : (crc << 1);
		}
	}

	return crc & 0xFF;	// & 0xFF для случая 16bit char
}

/*
  Name  : CRC-16 CCITT
  Poly  : 0x1021    x^16 + x^12 + x^5 + 1
  Init  : 0xFFFF
  Revert: false
  XorOut: 0x0000
  Check : 0x29B1 ("123456789")
  MaxLen: 4095 байт (32767 бит) - обнаружение
    одинарных, двойных, тройных и всех нечетных ошибок
*/
uint16_t Crc16(uint8_t *pcBlock, int len) {
	uint16_t crc = 0xFFFF;

	while (len--) {
		crc ^= *pcBlock++ << 8;

		for (int i = 0; i < 8; ++i) {
			crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : (crc << 1);
		}
    }

    return crc;
}

// Simple check sum MOD256
uint8_t CheckSum(uint8_t *data, int len) {
	uint8_t sum = 0;

	while (len--) {
		sum += *data++;
	}

	return sum & 0xFF;
}
