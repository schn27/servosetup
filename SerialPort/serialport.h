#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <stdint.h>

class SerialPort {
public:
	virtual ~SerialPort() {}
	
	virtual bool lock() = 0;
	virtual void unlock() = 0;
	virtual uint16_t read(void *buf, uint16_t size) = 0;
	virtual void write(const void *buf, uint16_t size) = 0;
	virtual void clean() = 0;
};

#endif
