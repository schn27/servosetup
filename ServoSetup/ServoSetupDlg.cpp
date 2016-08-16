#include "stdafx.h"
#include <string>
#include <sstream>
#include "ServoSetup.h"
#include "ServoSetupDlg.h"
#include "../SerialPort/serialport.h"
#include "connection.h"
#include "DataXchg.h"
#include "Logger.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CServoSetupDlg::CServoSetupDlg(CWnd* pParent /*=NULL*/)
		: CDialog(CServoSetupDlg::IDD, pParent)
		, dataXchg_(NULL)
		, logger_(NULL)
		, rs_(NULL)
		, path_initialized_(false) {
	hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CServoSetupDlg::~CServoSetupDlg() {
	delete dataXchg_;
	delete logger_;
	delete rs_;
}

void CServoSetupDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SETPOINT, setPoint);
	
	DDX_Control(pDX, IDC_PRESET, presetCtrl);
	DDX_Text(pDX, IDC_PRESET, preset);
	DDX_Control(pDX, IDC_SERVOSELECT, servoSelectCtrl);
	DDX_Text(pDX, IDC_SERVOSELECT, servoSelect);
	DDX_Control(pDX, IDC_SERVOSET, servoSetCtrl);
	DDX_Text(pDX, IDC_SERVOSET, servoSet);

	DDX_Control(pDX, IDC_CONNECT, buttonConnect);
	DDX_Control(pDX, IDC_GOODERR, goodErr);

	DDX_Control(pDX, IDC_INFO, curInfo);
	DDX_Control(pDX, IDC_CURPOSITION, curPosition);
	DDX_Control(pDX, IDC_CURSPEED, curSpeed);
	DDX_Control(pDX, IDC_CURUIN, curUin);
	DDX_Control(pDX, IDC_CURIOUT, curIout);
	DDX_Control(pDX, IDC_STOREINFILE, enableLog);

	DDX_Control(pDX, IDC_CONFIG, paramList_);
	DDX_Control(pDX, IDC_CONFIGEDITOR, valueEditor_);
}

BEGIN_MESSAGE_MAP(CServoSetupDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_CONNECT, &CServoSetupDlg::OnButtonConnect)
	ON_BN_CLICKED(IDC_CLOSE, &CServoSetupDlg::OnBnClickedClose)
	ON_BN_CLICKED(IDC_CFGWRITE, &CServoSetupDlg::OnBnClickedCfgwrite)
	ON_BN_CLICKED(IDC_CFGREAD, &CServoSetupDlg::OnBnClickedCfgread)
	ON_BN_CLICKED(IDC_ADDRWR, &CServoSetupDlg::OnBnClickedAddrwr)
	ON_BN_CLICKED(IDC_GOEND1, &CServoSetupDlg::OnBnClickedGoend1)
	ON_BN_CLICKED(IDC_GOCENTER, &CServoSetupDlg::OnBnClickedGocenter)
	ON_BN_CLICKED(IDC_GOEND2, &CServoSetupDlg::OnBnClickedGoend2)
	ON_BN_CLICKED(IDC_MANBEGIN, &CServoSetupDlg::OnBnClickedManbegin)
	ON_BN_CLICKED(IDC_MANSETEND1, &CServoSetupDlg::OnBnClickedMansetend1)
	ON_BN_CLICKED(IDC_MANSETCENTER, &CServoSetupDlg::OnBnClickedMansetcenter)
	ON_BN_CLICKED(IDC_MANSETEND2, &CServoSetupDlg::OnBnClickedMansetend2)
	ON_BN_CLICKED(IDC_MANEND, &CServoSetupDlg::OnBnClickedManend)
	ON_WM_TIMER()
	ON_CBN_SELCHANGE(IDC_PRESET, &CServoSetupDlg::OnCbnSelchangePreset)
	ON_CBN_SELCHANGE(IDC_SERVOSELECT, &CServoSetupDlg::OnCbnSelchangeServoselect)
END_MESSAGE_MAP()

BOOL CServoSetupDlg::OnInitDialog() {
	CDialog::OnInitDialog();

	SetIcon(hIcon, TRUE);		// Set big icon
	SetIcon(hIcon, FALSE);		// Set small icon

	initServoList();
	initParamList();

	setPoint.SetRange(-32767, 32767, TRUE);
	setPoint.SetPageSize(10);

	return TRUE;
}

void CServoSetupDlg::initServoList() {
	servolist_.read(getPath() + "config.xml");

	presetCtrl.ResetContent();

	std::vector<std::string> names;
	servolist_.getPresetNames(names);
	
	for (std::vector<std::string>::const_iterator p = names.begin(), end = names.end(); p != end; ++p) {
		presetCtrl.AddString(p->c_str());
	}

	presetCtrl.SetCurSel(0);
	UpdateData(TRUE);

	updateServoPreset(preset.GetBuffer());
}

void CServoSetupDlg::updateServoPreset(const std::string &preset) {
	servoSelectCtrl.ResetContent();
	servoSetCtrl.ResetContent();
	
	std::vector<std::string> names;
	servolist_.getServoNames(preset, names);
	
	for (std::vector<std::string>::const_iterator s = names.begin(), end = names.end(); s != end; ++s) {
		servoSelectCtrl.AddString(s->c_str());
		servoSetCtrl.AddString(s->c_str());
	}

	servoSelectCtrl.SetCurSel(0);
	servoSetCtrl.SetCurSel(0);

	UpdateData(TRUE);
}

void CServoSetupDlg::initParamList() {
	paramconfig_.read(getPath() + "config.xml");

	paramList_.Reset();
	paramList_.SetImageList(&imgList_, LVSIL_SMALL);
	paramList_.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	paramList_.InsertColumn(0, "ID", LVCFMT_LEFT, 40);
	paramList_.InsertColumn(1, "Name", LVCFMT_LEFT, 128);
	paramList_.InsertColumn(2, "Value", LVCFMT_LEFT, 80);

	paramList_.SetColumnEditor(2, &valueEditor_);

	for (int i = 0; i < paramconfig_.getNumOfParams(); ++i) {
		const ParamConfig::Param &param = paramconfig_.getParam(i);

		std::stringstream ss;
		ss << param.id;
		paramList_.InsertItem(i, ss.str().c_str());
		paramList_.SetItemText(i, 1, param.name.c_str());
		paramList_.SetItemText(i, 2, "");
		paramList_.SetRowColors(i, param.color, 0);
	}
}

void CServoSetupDlg::OnPaint() {
	if (IsIconic()) {
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, hIcon);
	} else {
		CDialog::OnPaint();
	}
}

HCURSOR CServoSetupDlg::OnQueryDragIcon() {
	return static_cast<HCURSOR>(hIcon);
}

void CServoSetupDlg::OnClose() {
	CDialog::OnClose();
}

void CServoSetupDlg::OnButtonConnect() {
	std::string str("Disconnect");

	if (dataXchg_ == NULL) {
		UpdateData(TRUE);

		delete rs_;
		rs_ = NULL;
		
		Connection connection = Connection(getPath() + "config.xml");
		rs_ = connection.getIf();

		const ServoList::Servo &servo = servolist_.getByName(preset.GetBuffer(), servoSelect.GetBuffer());
		
		dataXchg_ = new DataXchg(rs_, servo.addr, servo.transit);
		
		if (enableLog.GetCheck()) {
			SYSTEMTIME tm;
			GetSystemTime(&tm);
			CString str;
			str.Format("%02d%02d%02d_%02d%02d%02d", tm.wYear % 100, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);
			logger_ = new Logger(getPath() + str.GetString() + ".csv");

			const uint8_t filterIds[] = {0x20, 0x21, 0x30, 0x31, 0x32, 0x33};
			std::vector<uint8_t> filter = std::vector<uint8_t>(filterIds, filterIds + sizeof(filterIds));
			dataXchg_->enableLog(getPath() + str.GetString() + ".log", &filter);
		}

		OnBnClickedCfgread();

		presetCtrl.EnableWindow(0);
		servoSelectCtrl.EnableWindow(0);
		enableLog.EnableWindow(0);

		SetTimer(0, 20, NULL);
	} else {
		str = "Connect";

		delete dataXchg_;
		dataXchg_ = NULL;

		delete rs_;
		rs_ = NULL;
		
		delete logger_;
		logger_ = NULL;

		presetCtrl.EnableWindow();
		servoSelectCtrl.EnableWindow();
		enableLog.EnableWindow();

		KillTimer(0);
	}

	buttonConnect.SetWindowText(str.c_str());
}

void CServoSetupDlg::OnBnClickedClose() {
	EndDialog(0);
}

// установка бегунка в положение -100%
void CServoSetupDlg::OnBnClickedGoend1() {
	setPoint.SetPos(setPoint.GetRangeMin());
}

// установка бегунка в положение 0%
void CServoSetupDlg::OnBnClickedGocenter() {
	setPoint.SetPos(0);
}

// установка бегунка в положение +100%
void CServoSetupDlg::OnBnClickedGoend2() {
	setPoint.SetPos(setPoint.GetRangeMax());
}

// передача положения бегунка по таймеру
void CServoSetupDlg::OnTimer(UINT_PTR nIDEvent) {
	updateSetPoint();
	updatePacketCounter();
	updateParams();
	updateActualData();

	CDialog::OnTimer(nIDEvent);
}

void CServoSetupDlg::updateSetPoint() {
	if (dataXchg_ == NULL) {
		return;
	}

	dataXchg_->setPosition(setPoint.GetPos());
}

void CServoSetupDlg::updatePacketCounter() {
	if (dataXchg_ == NULL) {
		return;
	}

	int cntGood = dataXchg_->getCntGood();
	int cntBad = dataXchg_->getCntBad();
	CString str;
	str.Format("%d/%d", cntGood, cntBad);
	goodErr.SetWindowText(str);
}

void CServoSetupDlg::updateActualData() {
	if (dataXchg_ == NULL) {
		return;
	}

	const std::string & info = dataXchg_->getInfo();
	curInfo.SetWindowText(info.c_str());

	float position = 0;
	float speed = 0;
	float uin = 0;
	float iout = 0;

	dataXchg_->getStatus(position, speed, uin, iout);

	CString str;
	str.Format("%.1f", position);
	curPosition.SetWindowText(str);
	str.Format("%.1f", speed);
	curSpeed.SetWindowText(str);
	str.Format("%.1f", uin);
	curUin.SetWindowText(str);
	str.Format("%.1f", iout);
	curIout.SetWindowText(str);

	if (logger_ != NULL) {
		logger_->write(position, speed, uin, iout);
	}
}

// изменение параметров
void CServoSetupDlg::OnBnClickedCfgwrite() {
	if (dataXchg_ == NULL) {
		return;
	}

	UpdateData(TRUE);

	for (int i = 0, n = paramconfig_.getNumOfParams(); i < n; ++i) {
		const ParamConfig::Param &param = paramconfig_.getParam(i);

		std::istringstream ss(std::string(paramList_.GetItemText(i, 2)));

		double v = 0.0;
		ss >> v;

		int16_t newval = static_cast<int16_t>(min(max(v * (1 << param.fixedPoint), -32767), 32767));

		int16_t val;
		dataXchg_->getParam(param.id, val);

		if (val != newval) {
			dataXchg_->setParam(param.id, newval);
		}
	}

}

// чтение параметров
void CServoSetupDlg::OnBnClickedCfgread() {
	if (dataXchg_ == NULL) {
		return;
	}

	for (int i = 0, n = paramconfig_.getNumOfParams(); i < n; ++i) {
		dataXchg_->reqParam(paramconfig_.getParam(i).id);
		paramList_.SetItemText(i, 2, "");
	}
}

void CServoSetupDlg::updateParams() {
	if (dataXchg_ == NULL) {
		return;
	}

	for (int i = 0, n = paramconfig_.getNumOfParams(); i < n; ++i) {
		const ParamConfig::Param &param = paramconfig_.getParam(i);

		int16_t val;
		if (dataXchg_->getParam(param.id, val)) {
			std::stringstream ss;
			ss << static_cast<double>(val) / (1 << param.fixedPoint);
			paramList_.SetItemText(i, 2, ss.str().c_str());
		}
	}
}

void CServoSetupDlg::OnBnClickedAddrwr() {
	if (dataXchg_ == NULL) {
		return;
	}

	UpdateData(TRUE);
	const ServoList::Servo &servo = servolist_.getByName(preset.GetBuffer(), servoSet.GetBuffer());
	dataXchg_->setAddrCfg(servo.addr, servo.group);
}

void CServoSetupDlg::OnBnClickedManbegin() {
	if (dataXchg_ == NULL) {
		return;
	}

	dataXchg_->manualCfg(0);
}

void CServoSetupDlg::OnBnClickedMansetend1() {
	if (dataXchg_ == NULL) {
		return;
	}

	dataXchg_->manualCfg(1);
}

void CServoSetupDlg::OnBnClickedMansetcenter() {
	if (dataXchg_ == NULL) {
		return;
	}

	dataXchg_->manualCfg(2);
}

void CServoSetupDlg::OnBnClickedMansetend2() {
	if (dataXchg_ == NULL) {
		return;
	}

	dataXchg_->manualCfg(3);
}

void CServoSetupDlg::OnBnClickedManend() {
	if (dataXchg_ == NULL) {
		return;
	}

	dataXchg_->manualCfg(4);

	OnBnClickedCfgread();
}

const std::string & CServoSetupDlg::getPath() {
	if (!path_initialized_) {
		char szAppPath[MAX_PATH] = "";
		GetModuleFileName(0, szAppPath, sizeof(szAppPath) - 1);

		path_ = szAppPath;
		path_ = path_.substr(0, path_.rfind("\\")).append("\\");

		path_initialized_ = true;
	}

	return path_;
}

void CServoSetupDlg::OnCbnSelchangePreset() {
	UpdateData(TRUE);
	updateServoPreset(preset.GetBuffer());
}

void CServoSetupDlg::OnCbnSelchangeServoselect() {
	UpdateData(TRUE);
	servoSetCtrl.SelectString(-1, servoSelect);
	UpdateData(TRUE);
}
