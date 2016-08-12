#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include <string>
#include "../CustomCtrl/ListCtrlEx.h"
#include "DataXchg.h"
#include "Logger.h"
#include "paramconfig.h"
#include "servolist.h"

class SeriaPort;

class CServoSetupDlg : public CDialog {
public:
	CServoSetupDlg(CWnd* pParent = NULL);
	~CServoSetupDlg();	

	enum { IDD = IDD_SERVOPRGUI_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

protected:
	HICON hIcon;

	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonConnect();
	afx_msg void OnClose();
	afx_msg void OnBnClickedClose();
	afx_msg void OnBnClickedAddrwr();
	afx_msg void OnBnClickedCfgwrite();
	afx_msg void OnBnClickedCfgread();
	afx_msg void OnBnClickedGoend1();
	afx_msg void OnBnClickedGocenter();
	afx_msg void OnBnClickedGoend2();
	afx_msg void OnBnClickedManbegin();
	afx_msg void OnBnClickedMansetend1();
	afx_msg void OnBnClickedMansetcenter();
	afx_msg void OnBnClickedMansetend2();
	afx_msg void OnBnClickedManend();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()

	CSliderCtrl setPoint;
	CButton enableLog;

	CComboBox presetCtrl;
	CString preset;
	CComboBox servoSelectCtrl;
	CString servoSelect;
	CComboBox servoSetCtrl;
	CString servoSet;

	CButton buttonConnect;
	CEdit goodErr;
	CEdit curInfo;
	CEdit curPosition;
	CEdit curSpeed;
	CEdit curUin;
	CEdit curIout;

private:
	void updateSetPoint();
	void updatePacketCounter();
	void updateConfig();
	void updateActualData();
	void updateParams();

	void initServoList();
	void updateServoPreset(const std::string &preset);
	void initParamList();

	const std::string & getPath();

	CImageList imgList_;
	CListCtrlEx paramList_;
	CEdit valueEditor_;

	ParamConfig paramconfig_;
	ServoList servolist_;
	DataXchg *dataXchg_;
	Logger logger_;

	SerialPort *rs_;

	bool path_initialized_;
	std::string path_;

public:
	afx_msg void OnCbnSelchangePreset();
	afx_msg void OnCbnSelchangeServoselect();
};

