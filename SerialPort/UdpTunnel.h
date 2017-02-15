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
	virtual size_t read(void *buf, size_t size);
	virtual void write(const void *buf, size_t size);
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

	const size_t bufferSize_;
	uint8_t *buffer_;
	size_t bufferPos_;
	size_t dataInBuffer_;

	const size_t timeout_;

	static bool wsastartup_;
};

#endif
