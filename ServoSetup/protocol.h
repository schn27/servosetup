#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <ostream>
#include <string>
#include <vector>

class SerialPort;

class Protocol {
public:
	Protocol(SerialPort *rs, uint8_t *buffer, size_t size, const std::string &logName = "", const std::vector<uint8_t> *filter = NULL);
	~Protocol();
	
	uint8_t *getDataPointer();
	size_t getMaxDataSize() const;
	
	bool send(uint8_t addr, uint8_t id, size_t size);
	bool send(uint8_t addr, uint8_t id, const uint8_t *data, size_t size);	
	bool receive(uint8_t &addr, uint8_t &id, size_t &size);
	bool receiveResponse(uint8_t addr, uint8_t id, size_t &size);
	
private:
	std::string getTimeStr() const;
	std::string bytesToStr(const uint8_t *data, size_t size) const;
	bool isLogRequired(uint8_t id) const;

	SerialPort *rs_;
	uint8_t *buffer_;
	const size_t buffer_size_;

	std::ostream *log_;
	std::vector<uint8_t> filter_;
};


#endif
