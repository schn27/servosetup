#include <string.h>
#include <cassert>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <windows.h>
#include "../SerialPort/serialport.h"
#include "crc.h"
#include "protocol.h"

Protocol::Protocol(SerialPort *rs, uint8_t *buffer, size_t size, const std::string &logName, const std::vector<uint8_t> *filter)
		: rs_(rs)
		, buffer_(buffer)
		, buffer_size_(size)
		, log_(logName.empty() ? nullptr : new std::ofstream(logName, std::fstream::out | std::fstream::app))
		, filter_(filter != nullptr ? *filter : std::vector<uint8_t>()) {
	assert(buffer_size_ >= 4);
}

Protocol::~Protocol() {
	delete log_;
}

uint8_t *Protocol::getDataPointer() {
	return buffer_ + 3;
}

size_t Protocol::getMaxDataSize() const {
	return buffer_size_ - 4;
}

// формирование и отправка пакета
bool Protocol::send(uint8_t addr, uint8_t id, size_t size) {
	buffer_[0] = addr;
	buffer_[1] = id;
	buffer_[2] = uint8_t(size + 4);
	buffer_[buffer_[2] - 1] = crc::crc8(buffer_, buffer_[2] - 1);

	rs_->write(buffer_, buffer_[2]);

	if (log_ != nullptr && isLogRequired(buffer_[1])) {
		*log_ << std::endl << getTimeStr() << " " << "out " << bytesToStr(buffer_, buffer_[2]) << std::endl;
	}

	return true;
}

// формирование и отправка пакета (с копированием поля данных)
bool Protocol::send(uint8_t addr, uint8_t id, const uint8_t *data, size_t size) {
	if (size > buffer_size_ - 4) {
		return false;
	}

	memcpy(buffer_ + 3, data, size);
	
	return send(addr, id, size);
}

// приём пакета
bool Protocol::receive(uint8_t &addr, uint8_t &id, size_t &size) {
	int n = rs_->read(buffer_, 4);

	if (n < 4 || buffer_[2] > buffer_size_ || buffer_[2] < 4) {
		if (log_ != nullptr) {
			*log_ << getTimeStr() << " " << "in TIMEOUT " << bytesToStr(buffer_, n) << std::endl;
		}
		return false;
	}

	if (buffer_[2] > 4) {
		int n = rs_->read(buffer_ + 4, buffer_[2] - 4);

		if ((n < buffer_[2] - 4) || (crc::crc8(buffer_, buffer_[2] - 1) != buffer_[buffer_[2] - 1])) {
			if (log_ != nullptr) {
				*log_ << getTimeStr() << " " << "in BAD " << bytesToStr(buffer_, n) << std::endl;
			}
			return false;
		}
	}

	addr = buffer_[0];
	id = buffer_[1];
	size = buffer_[2] - 4;

	if (log_ != nullptr && isLogRequired(buffer_[1])) {
		*log_ << getTimeStr() << " " << "in  " << bytesToStr(buffer_, buffer_[2]) << std::endl;
	}

	return true;
}

// приём пакета с проверкой addr и id
bool Protocol::receiveResponse(uint8_t addr, uint8_t id, size_t &size) {
	uint8_t addr_recv = 0;
	uint8_t id_recv = 0;
	
	if (!receive(addr_recv, id_recv, size)) {
		return false;
	}
	
	return addr_recv == addr && id_recv == id;
}

std::string Protocol::getTimeStr() const {
	SYSTEMTIME st; 
	GetLocalTime(&st);
	std::stringstream s;
	s << st.wHour << ":" << st.wMinute << ":" << st.wSecond << "." << std::setw(3) << std::setfill('0') << st.wMilliseconds;
	return s.str();
}

std::string Protocol::bytesToStr(const uint8_t *data, size_t size) const {
	std::stringstream s;
	
	for (size_t i = 0; i < size; ++i) {
		s << std::hex << std::setw(2) << std::setfill('0') << (int)data[i] << " ";
	}

	return s.str();
}

bool Protocol::isLogRequired(uint8_t id) const {
	return filter_.empty() || std::find(filter_.begin(), filter_.end(), id) != filter_.end();
}