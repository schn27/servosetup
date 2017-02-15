#include <string>
#include "Rs232.h"

Rs232::Rs232(const char *name, size_t baudRate, size_t timeOut)
		: name_(name)
		, baudRate_(baudRate)
		, timeOut_(timeOut)
		, initialized_(false)
		, port_(INVALID_HANDLE_VALUE) {
}

Rs232::~Rs232() {
	deinit();
}

bool Rs232::lock() {
	return true;
}

void Rs232::unlock() {
}

size_t Rs232::read(void *buf, size_t size) {
	if (init()) {
		uint32_t result = 0;
		return ReadFile(port_, buf, size, reinterpret_cast<LPDWORD>(&result), nullptr) ? result : 0;
	} else {
		Sleep(timeOut_);
		return 0;
	}
}

void Rs232::write(const void *buf, size_t size) {
	if (init()) {
		uint32_t junk = 0;
		WriteFile(port_, buf, size, reinterpret_cast<LPDWORD>(&junk), nullptr);
	}
}

void Rs232::clean() {
	if (init()) {
		PurgeComm(port_, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
	}
}

bool Rs232::init() {
	if (initialized_) {
		return true;
	}

	std::string tmp("\\\\.\\");
	tmp += name_;

	port_ = CreateFile(tmp.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);

	if (port_ == INVALID_HANDLE_VALUE) {
		return false;
	}

	DCB dcb = {0};
	dcb.DCBlength = sizeof dcb;
	
	bool success = GetCommState(port_, &dcb) == TRUE;
	
	dcb.BaudRate = baudRate_;
	dcb.ByteSize = 8;
	dcb.fParity = FALSE;
	dcb.StopBits = ONESTOPBIT;
	dcb.fBinary = TRUE;
	dcb.fAbortOnError = FALSE;
	
	success &= SetCommState(port_, &dcb) == TRUE;

	COMMTIMEOUTS ct = {0};
	ct.ReadIntervalTimeout = timeOut_;
	ct.ReadTotalTimeoutConstant = 100;
	ct.ReadTotalTimeoutMultiplier = timeOut_;
	ct.WriteTotalTimeoutConstant = 10;
	ct.WriteTotalTimeoutMultiplier = 1;
	
	success &= SetCommTimeouts(port_, &ct) == TRUE;

	success &= SetupComm(port_, 1024, 1024) == TRUE;

	if (!success) {
		deinit();
		return false;
	}
	
	initialized_ = true;
	return true;
}

void Rs232::deinit() {
	if (port_ != INVALID_HANDLE_VALUE) {
		CloseHandle(port_);
		port_ = INVALID_HANDLE_VALUE;
	}
}
