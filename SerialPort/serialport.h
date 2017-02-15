#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <stdint.h>

class SerialPort {
public:
	virtual ~SerialPort() {}
	
	virtual bool lock() = 0;
	virtual void unlock() = 0;
	virtual size_t read(void *buf, size_t size) = 0;
	virtual void write(const void *buf, size_t size) = 0;
	virtual void clean() = 0;
};

#endif
