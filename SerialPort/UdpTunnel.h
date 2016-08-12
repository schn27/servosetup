#ifndef UDPTUNNEL_H
#define UDPTUNNEL_H

#include <string>
#include <winsock.h>
#include "serialport.h"

class UdpTunnel : public SerialPort {
public:
	UdpTunnel(const char *selfIp, uint16_t selfPort, const char *remoteIp, uint16_t remotePort);
	virtual ~UdpTunnel();

	virtual bool lock();
	virtual void unlock();
	virtual uint16_t read(void *buf, uint16_t size);
	virtual void write(const void *buf, uint16_t size);
	virtual void clean();

private:
	UdpTunnel(const UdpTunnel &);

	void init();
	void deinit();

	bool initialized_;

	std::string selfIp_;
	uint16_t selfPort_;
	std::string remoteIp_;
	uint16_t remotePort_;
	in_addr remote_addr_;
	bool update_remote_addr_;

	SOCKET sck_;

	const uint32_t bufferSize_;
	uint8_t *buffer_;
	uint32_t bufferPos_;
	uint32_t dataInBuffer_;

	const uint32_t timeout_;

	static bool wsastartup_;
};

#endif
