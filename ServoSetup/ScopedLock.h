#ifndef SCOPEDLOCK_H
#define SCOPEDLOCK_H

#include "stdafx.h"

template <typename T>
struct ScopedLock {
	ScopedLock(T &lock) : lock_(lock){
		lock_.Lock();
	}

	~ScopedLock() {
		lock_.Unlock();
	}
private:
	T &lock_;
};

#endif
