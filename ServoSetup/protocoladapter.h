#ifndef PROTOCOLADAPTER_H
#define PROTOCOLADAPTER_H

#include <stdint.h>
#include "../SerialPort/serialport.h"

class ProtocolAdapter : public SerialPort {
public:
	ProtocolAdapter(SerialPort *port, uint8_t addr, uint8_t id, uint8_t *buffer, size_t size);
	virtual ~ProtocolAdapter();
	
	virtual bool lock() {
		return true;
	}

	virtual void unlock() {
	}

	virtual size_t read(void *buf, size_t size);
	virtual void write(const void *buf, size_t size);
	
	virtual void clean() {
	}

private:
	SerialPort *port_;
	const uint8_t id_;
	const uint8_t addr_;
	uint8_t *buffer_;
	const size_t buffer_size_;
	bool empty_;	/* true if no packet in buffer */
	size_t size_;	/* received packet size */
	uint8_t *read_;	/* read pointer */
};

#endif
