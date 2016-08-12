#include "protocoladapter.h"
#include "protocol.h"

ProtocolAdapter::ProtocolAdapter(SerialPort *port, uint8_t addr, uint8_t id, uint8_t *buffer, int size)
		: port_(port)
		, addr_(addr)
		, id_(id)
		, buffer_(buffer)
		, buffer_size_(size)
		, empty_(true)
		, read_(buffer) {
}

ProtocolAdapter::~ProtocolAdapter() {
}

/**
 * Reads packet from 'rs_' if 'empty_' flag is set.
 * Copies 'size' bytes from 'read_' to 'data'. Then increments 'read_'.
 */
uint16_t ProtocolAdapter::read(void *data, uint16_t size) {
	if (empty_) {
		Protocol protocol(port_, buffer_, buffer_size_);
		empty_ = !protocol.receiveResponse(addr_, id_ + 1, size_);
		read_ = protocol.getDataPointer();
		
		if (empty_) {
			return 0;
		}
	}

	if (size > size_) {
		size = size_;
		empty_ = true;
	}

	memcpy(data, read_, size);
	
	read_ += size;
	size_ -= size;

	return size;
}

/**
 * Wraps 'data' in packet using 'addr_' & 'id_' and sends it to 'rs_'
 */
void ProtocolAdapter::write(const void *data, uint16_t size) {
	Protocol protocol(port_, buffer_, buffer_size_);
	protocol.send(addr_, id_, static_cast<const uint8_t *>(data), size);
}
