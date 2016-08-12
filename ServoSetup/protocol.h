#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <ostream>
#include <string>

class SerialPort;

class Protocol {
public:
	Protocol(SerialPort *rs, uint8_t *buffer, int size, const std::string &logName = "");
	~Protocol();
	
	uint8_t *getDataPointer();
	int getMaxDataSize() const;
	
	bool send(uint8_t addr, uint8_t id, int size);
	bool send(uint8_t addr, uint8_t id, const uint8_t *data, int size);	
	bool receive(uint8_t &addr, uint8_t &id, int &size);
	bool receiveResponse(uint8_t addr, uint8_t id, int &size);
	
private:
	std::string getTimeStr() const;
	std::string bytesToStr(const uint8_t *data, int size) const;

	SerialPort *rs_;
	uint8_t *buffer_;
	const int buffer_size_;

	std::ostream *log_;
};


#endif
