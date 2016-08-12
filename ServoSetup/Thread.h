#ifndef THREAD_H
#define THREAD_H

#include <windows.h>

class Thread {
public:
	Thread();
	virtual ~Thread();
	int start();
	void stop();

protected:
	static const UINT msgBase = WM_USER + 1;

	void post(UINT msg, WPARAM wParam = 0, LPARAM lParam = 0);
	bool isStopping() const;

private:
	virtual int threadProc(void *p) = 0;
	
	static int WINAPI execute(void *ptr) {
		return static_cast<Thread *>(ptr)->threadProc(static_cast<Thread *>(ptr)->param_);
	}

	HANDLE handle_;
	DWORD id_;
	void *param_;
	volatile bool stopping_;
};

#endif
