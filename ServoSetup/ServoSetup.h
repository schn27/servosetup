#pragma once
#include "afxcmn.h"
#include "afxwin.h"

#include "resource.h"		// main symbols

class CServoSetup : public CWinApp {
public:
	CServoSetup();

	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()

private:
	HANDLE mutex_;
};

extern CServoSetup theApp;
