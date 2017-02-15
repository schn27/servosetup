#ifndef NULLSERIALPORT_H
#define NULLSERIALPORT_H

#include "serialport.h"

class NullSerialPort : public SerialPort {
public:
	NullSerialPort() {
	}
	
	virtual ~NullSerialPort() {
	}
	
	virtual bool lock() {
		return true;
	}
	
	virtual void unlock() {
	}
	
	virtual size_t read(void *buf, size_t size) {
		return 0;
	}
	
	virtual void write(const void *buf, size_t size) {
	}
	
	virtual void clean() {
	}

private:
	NullSerialPort(const NullSerialPort &);
};

#endif
