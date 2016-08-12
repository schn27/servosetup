#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

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
