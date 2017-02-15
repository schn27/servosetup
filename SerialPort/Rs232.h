#ifndef RS232_H
#define RS232_H

#include <stdint.h>
#include <string>
#include <windows.h>
#include "serialport.h"

class Rs232 : public SerialPort
{
public:
	Rs232(const char *name, size_t baudRate, size_t timeOut);
	virtual ~Rs232();
	virtual bool lock();
	virtual void unlock();
	virtual size_t read(void *buf, size_t size);
	virtual void write(const void *buf, size_t size);
	virtual void clean();

private:
	Rs232(const Rs232 &);
	
	bool init();
	void deinit();

	const std::string name_;
	const size_t baudRate_;
	const size_t timeOut_;
	bool initialized_;
	HANDLE port_;	// Port handle, INVALID_HANDLE_VALUE if port is not opened
};

#endif
