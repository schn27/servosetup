#include "ServoSetup.h"
#include "ServoSetupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CServoSetup, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CServoSetup theApp;

CServoSetup::CServoSetup() {
}

BOOL CServoSetup::InitInstance() {
	mutex_ = CreateMutex(NULL, FALSE, _T("schn27.servosetup"));
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		return FALSE;
	}
	
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	CServoSetupDlg dlg;
	m_pMainWnd = &dlg;

	INT_PTR nResponse = dlg.DoModal();

	CloseHandle(mutex_);

	return FALSE;
}
