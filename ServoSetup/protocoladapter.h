#ifndef PROTOCOLADAPTER_H
#define PROTOCOLADAPTER_H

#include <stdint.h>
#include "../SerialPort/serialport.h"

class ProtocolAdapter : public SerialPort {
public:
	ProtocolAdapter(SerialPort *port, uint8_t addr, uint8_t id, uint8_t *buffer, int size);
	virtual ~ProtocolAdapter();
	
	virtual bool lock() {
		return true;
	}

	virtual void unlock() {
	}

	virtual uint16_t read(void *buf, uint16_t size);
	virtual void write(const void *buf, uint16_t size);
	
	virtual void clean() {
	}

private:
	SerialPort *port_;
	const uint8_t id_;
	const uint8_t addr_;
	uint8_t *buffer_;
	const int buffer_size_;
	bool empty_;	/* true if no packet in buffer */
	int size_;	/* received packet size */
	uint8_t *read_;	/* read pointer */
};

#endif
