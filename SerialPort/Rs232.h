#ifndef RS232_H
#define RS232_H

#include <stdint.h>
#include <string>
#include <windows.h>
#include "serialport.h"

class Rs232 : public SerialPort
{
public:
	Rs232(const char *name, uint32_t baudRate, uint16_t timeOut);
	virtual ~Rs232();
	virtual bool lock();
	virtual void unlock();
	virtual uint16_t read(void *buf, uint16_t size);
	virtual void write(const void *buf, uint16_t size);
	virtual void clean();

private:
	Rs232(const Rs232 &);
	
	void init();
	void deinit();

	std::string name_;
	uint32_t baudRate_;
	uint16_t timeOut_;
	bool initialized_;
	HANDLE port_;	// Port handle, INVALID_HANDLE_VALUE if port is not opened
};

#endif
