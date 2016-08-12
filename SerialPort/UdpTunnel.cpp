#include <cassert>
#include <iostream>
#include "UdpTunnel.h"

#pragma comment(lib, "ws2_32.lib")

bool UdpTunnel::wsastartup_ = false;

UdpTunnel::UdpTunnel(const char *selfIp, uint16_t selfPort, const char *remoteIp, uint16_t remotePort)
		: initialized_(false)
		, selfIp_(selfIp)
		, selfPort_(selfPort)
		, remoteIp_(remoteIp)
		, remotePort_(remotePort)
		, update_remote_addr_(false)
		, sck_(NULL)
		, bufferSize_(1024)
		, buffer_(new uint8_t[bufferSize_])
		, bufferPos_(0)
		, dataInBuffer_(0)
		, timeout_(200) {
	assert(bufferSize_);
}

UdpTunnel::~UdpTunnel() {
	deinit();
	delete [] buffer_;
}


bool UdpTunnel::lock() {
	return true;
}

void UdpTunnel::unlock() {
}

uint16_t UdpTunnel::read(void *data, uint16_t size) {
	init();

	uint32_t result = 0;

	if (dataInBuffer_ >= size) {
		// если данных в буфере больше чем надо
		memcpy(data, buffer_ + bufferPos_, size);
		result = size;
		bufferPos_ += size;
		dataInBuffer_ -= size;
	} else {
		// если данных в буфере меньше чем надо
		
		uint8_t *dataPtr = (uint8_t *)data;

		if (dataInBuffer_) {
			// прочитать остатки

			memcpy(dataPtr, buffer_ + bufferPos_, dataInBuffer_);
			result = dataInBuffer_;
			dataPtr += dataInBuffer_;
			size -= dataInBuffer_;
		}

		while (size) {
			dataInBuffer_ = 0;
			bufferPos_ = 0;

			// принять новую датаграму

			fd_set set = {0};
			FD_SET(sck_, &set);
			TIMEVAL tv = {0, timeout_ * 1000}; // seconds, microseconds
			
			if (select(0, &set, NULL, NULL, &tv) > 0) {
				sockaddr_in sa = {0};
				int sa_size = sizeof(sa);
				int n = recvfrom(sck_, (char *)buffer_, bufferSize_, 0, (sockaddr *)&sa, &sa_size);
			
				if (update_remote_addr_) {
					remote_addr_ = sa.sin_addr;
				}

				dataInBuffer_ = max(n, 0);	// страховка от n < 0
				
				n = min(dataInBuffer_, size);	// читаем меньше или равно того что приняли
				memcpy(dataPtr, buffer_, n);
				dataPtr += n;
				result += n;
				bufferPos_ = n;
				dataInBuffer_ -= n;
				size -= n;
			} else {
				return result;
			}
		}
	}
	
	return result;
}

void UdpTunnel::write(const void *data, uint16_t size) {
	init();

	sockaddr_in sa = {0};
	sa.sin_family = AF_INET;
	sa.sin_port = htons(remotePort_);
	sa.sin_addr = remote_addr_;
	sendto(sck_, (const char *)data, size, 0, (sockaddr *)&sa, sizeof(sa));
}

void UdpTunnel::clean() {
	init();

	fd_set set = {0};
	FD_SET(sck_, &set);
	TIMEVAL tv = {0, 0}; // seconds, microseconds

	while (select(0, &set, NULL, NULL, &tv) > 0) {
		sockaddr_in sa = {0};
		int sa_size = sizeof(sa);
		int n = recvfrom(sck_, (char *)buffer_, bufferSize_, 0, (sockaddr *)&sa, &sa_size);
	}

	dataInBuffer_ = 0;
	bufferPos_ = 0;
}

void UdpTunnel::init() {
	if (initialized_) {
		return;
	}

	if (!wsastartup_) {
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			throw std::runtime_error("WSAStartup failed");
		}
		wsastartup_ = true;
	}

	remote_addr_.s_addr = inet_addr(remoteIp_.c_str());
	update_remote_addr_ = remoteIp_ == "0.0.0.0";

	sockaddr_in sa = {0};

	sa.sin_family = AF_INET;
	sa.sin_port = htons(selfPort_);
	sa.sin_addr.s_addr = inet_addr(selfIp_.c_str());

	sck_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sck_ == INVALID_SOCKET) {
		throw std::runtime_error("Socket creation failed");
	}

	const BOOL reuse_value = TRUE;
	setsockopt(sck_, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse_value, sizeof(reuse_value));
	
	if (bind(sck_, (sockaddr *)&sa, sizeof(sa)) != 0) {
		throw std::runtime_error("Socket binding failed");
	}

	initialized_ = true;
}


void UdpTunnel::deinit() {
	closesocket(sck_);
}
