#include "stdafx.h"
#include <cassert>
#include "Thread.h"

Thread::Thread() 
		: handle_(INVALID_HANDLE_VALUE)
		, stopping_(false) {
}

Thread::~Thread() {
	stop();
}

int Thread::start() {
	if (handle_ == INVALID_HANDLE_VALUE) {
		handle_ = CreateThread(
			NULL, 
			0, 
			reinterpret_cast<LPTHREAD_START_ROUTINE>(Thread::execute), 
			this, 
			0, 
			&id_);
	}

	return handle_ != INVALID_HANDLE_VALUE;
}

void Thread::stop() {
	if (handle_ != INVALID_HANDLE_VALUE) {
		stopping_ = true;
		WaitForSingleObject(handle_, INFINITE);
		handle_ = INVALID_HANDLE_VALUE;
	}
}

void Thread::post(UINT msg, WPARAM wParam, LPARAM lParam) {
	PostThreadMessage(id_, msg, wParam, lParam);
}

bool Thread::isStopping() const {
	return stopping_;
}
