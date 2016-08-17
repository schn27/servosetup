#include "stdafx.h"
#include "protocol.h"
#include "protocoladapter.h"
#include "DataXchg.h"
#include "ScopedLock.h"

namespace {

enum {
	idGetVer = 0x00,
	idGetState = 0x02,
	idSetPosNoAnswer = 0x10,
	idSetPos = 0x12,
	idSetRS485 = 0x20,
	idGetPar = 0x30,
	idSetPar = 0x32,
	idManualSetup = 0x38,
	idTransit = 0xF0
};

namespace servoconsts {
const float PositionScale = 90.0f / 1500.0f;
const float SpeedScale = 3;
const float UinScale = 0.012f;
const float IoutScale = 0.001f;
}

}

const int DataXchg::bufferSize_ = 256;
const int DataXchg::buffer2Size_ = 256 + 4;

DataXchg::DataXchg(SerialPort *rs, uint8_t addr, int transitAddr, bool broadcast)
		: rs_(rs)
		, buffer_(new uint8_t[bufferSize_])
		, buffer2_(new uint8_t[buffer2Size_])
		, addr_(addr)
		, transitAddr_(transitAddr)
		, cntGood_(0)
		, cntBad_(0)
		, cfgread_(false)
		, cmdPosition_(0)
		, position_(0)
		, speed_(0)
		, uin_(0)
		, iout_(0)
		, info_("")
		, noInfo_(true)
		, cfgAddr_(0)
		, cfgAddrAlias_(0)
		, manualActive_(false)
		, broadcast_(broadcast)
		, logName_("")
		, logFilter_(NULL) {
	params_.reserve(256);

	for (int i = 0; i < 256; ++i) {
		params_.push_back(param_t());
	}

	Thread::start();
}

DataXchg::~DataXchg() {
	Thread::stop();

	delete [] buffer_;
	delete [] buffer2_;
}

void DataXchg::getStatus(float &position, float &speed, float &uin, float &iout) {
	ScopedLock<CCriticalSection> _(criticalSection_);
	position = position_;
	speed = speed_;
	uin = uin_;
	iout = iout_;
}

void DataXchg::setPosition(int value) {
	ScopedLock<CCriticalSection> _(criticalSection_);
	cmdPosition_ = value;
}

void DataXchg::reqParam(size_t index) {
	if (index >= params_.size()) {
		return;
	}

	ScopedLock<CCriticalSection> _(criticalSection_);
	params_[index].state = eParReadReq;
}

bool DataXchg::getParam(size_t index, int16_t &value) {
	if (index >= params_.size()) {
		return false;
	}

	bool res = false;

	ScopedLock<CCriticalSection> _(criticalSection_);

	param_t &param = params_[index];
	value = param.value;

	if (param.state == eParReadResp) {
		param.state = eParIdle;
		res = true;
	}

	return res;
}

void DataXchg::setParam(size_t index, int16_t value) {
	if (index >= params_.size()) {
		return;
	}

	ScopedLock<CCriticalSection> _(criticalSection_);
	param_t &param = params_[index];
	param.value = value;
	param.state = eParWriteReq;
}

void DataXchg::setAddrCfg(uint8_t addr, uint8_t addrAlias) {
	cfgAddr_ = addr;
	cfgAddrAlias_ = addrAlias;
	post(msgWriteAddr);
}

void DataXchg::setAddr(uint8_t addr) {
	addr_ = addr;
	noInfo_ = true;
}

void DataXchg::manualCfg(uint8_t cmd) {
	post(msgManual, cmd);
}

int DataXchg::threadProc(void * /*p*/) {
	while (!isStopping()) {
		MSG msg;
		if (PeekMessage(&msg, reinterpret_cast<HWND>(-1), 0, 0, PM_REMOVE)) {
			switch (msg.message) {
			case msgWriteAddr: doWriteAddr(); break;
			case msgManual: doManual(msg.wParam); break;
			}
		}

		updateInfo();
		update();
		updateParams(true);
	}

	return 0;
}

void DataXchg::updateInfo() {
	if (!noInfo_ || broadcast_) {
		return;
	}

	uint8_t buf[256];
	uint8_t bufsize = (uint8_t)sizeof(buf);
	memset(buf, 0, bufsize);
	
	if (!request(addr_, idGetVer, buf, 0, buf, bufsize)) {
		return;
	}

	info_.assign(reinterpret_cast<const char *>(buf), bufsize);
	noInfo_ = false;
}

void DataXchg::update() {
	uint8_t buf[8];
	
	{
		ScopedLock<CCriticalSection> _(criticalSection_);
		buf[0] = cmdPosition_;
		buf[1] = cmdPosition_ >> 8;
		buf[2] = 3;
	}

	if (broadcast_) {
		requestNoAnswer(addr_, idSetPosNoAnswer, buf, 2);
		return;
	}

	if (!manualActive_) {
		uint8_t n = 0;
		request(addr_, idSetPos, buf, 2, NULL, n);

		if (transitAddr_ >= 0) {
			request(transitAddr_, idSetPos, buf, 3, NULL, n);
		}
	}

	uint8_t bufsize = 8;
	if (request(addr_, idGetState, buf, 0, buf, bufsize)) {
		ScopedLock<CCriticalSection> _(criticalSection_);
		position_ = static_cast<int16_t>(buf[0] + buf[1] * 256) * servoconsts::PositionScale;
		speed_ = static_cast<int16_t>(buf[2] + buf[3] * 256) * servoconsts::SpeedScale;
		uin_ = static_cast<uint16_t>(buf[4] + buf[5] * 256) * servoconsts::UinScale;
		iout_ = static_cast<uint16_t>(buf[6] + buf[7] * 256) * servoconsts::IoutScale;
	}
}

void DataXchg::updateParams(bool all) {
	bool updated = false;
	
	for (int i = 0, n = params_.size(); (i < n) && (!updated || all); ++i) {
		param_t param;
		{
			ScopedLock<CCriticalSection> _(criticalSection_);
			param = params_[i];
		}

		if (param.state == eParReadReq)	{
			if (readParam(i, param.value)) {
				param.state = eParReadResp;
	
				ScopedLock<CCriticalSection> _(criticalSection_);
				params_[i] = param;
			}

			updated = true;
		} else if (param.state == eParWriteReq) {
			if (writeParam(i, param.value)) {
				param.state = eParIdle;

				ScopedLock<CCriticalSection> _(criticalSection_);
				params_[i] = param;
			}

			updated = true;
		}
	}
}

bool DataXchg::writeParam(uint8_t id, int16_t value) {
	for (int i = 0; i < 3; ++i) {
		uint8_t buf[3] = {id, value & 0xFF, value >> 8};
		uint8_t n = 0;
		if (request(addr_, idSetPar, buf, 3, NULL, n)) {
			return true;
		}
	}

	return false;
}

bool DataXchg::readParam(uint8_t id, int16_t &value) {
	value = 0;

	for (int i = 0; i < 3; ++i) {
		uint8_t buf[2] = {id, 0};
		uint8_t bufsize = 2;
		if (request(addr_, idGetPar, buf, 1, buf, bufsize)) {
			value = static_cast<int16_t>(buf[0] + buf[1] * 256);
			return true;
		}
	}

	return false;
}

void DataXchg::doWriteAddr() {
	if (broadcast_) {
		return;
	}

	for (int i = 0; i < 3; ++i) {
		uint32_t rate = 115200;
		uint8_t buf[6] = {rate, rate >> 8, rate >> 16, rate >> 24, cfgAddr_, cfgAddrAlias_};
		uint8_t n = 0;
		if (request(addr_, idSetRS485, buf, 6, NULL, n)) {
			return;
		}
	}
}

void DataXchg::doManual(uint8_t cmd) {
	if (broadcast_) {
		return;
	}

	if (cmd == 0) {
		manualActive_ = true;
	} else if (cmd == 4) {
		manualActive_ = false;
	}

	for (int i = 0; i < 3; ++i) {
		uint8_t n = 0;
		if (request(addr_, idManualSetup, &cmd, 1, NULL, n)) {
			return;
		}
	}
}

// отправка запроса и приём ответа
bool DataXchg::request(uint8_t addr, uint8_t id, uint8_t *data, uint8_t datasize, uint8_t *response, uint8_t &responsesize) {
	if (isStopping()) {
		return false;
	}

	rs_->clean();

	ProtocolAdapter transit(rs_, transitAddr_, idTransit, buffer2_, buffer2Size_);
	Protocol protocol((transitAddr_ < 0 || addr == transitAddr_) ? rs_ : &transit, buffer_, bufferSize_, logName_, &logFilter_);
	
	if (!protocol.send(addr, id, data, datasize)) {
		return false;
	}
	
	uint8_t addr_recv;
	uint8_t id_recv;
	int size;

	if (!protocol.receive(addr_recv, id_recv, size) || (size != responsesize && responsesize < 252)) {
		++cntBad_;
		return false;
	}

	if (responsesize > size) {
		responsesize = size;
	}

	if (response != NULL) {
		memcpy(response, protocol.getDataPointer(), responsesize);
	}

	++cntGood_;

	return true;
}

// отправка запроса без ожидания ответа
bool DataXchg::requestNoAnswer(uint8_t addr, uint8_t id, uint8_t *data, uint8_t datasize) {
	if (isStopping()) {
		return false;
	}

	ProtocolAdapter transit(rs_, transitAddr_, idTransit, buffer2_, buffer2Size_);
	Protocol protocol((transitAddr_ < 0 || addr == transitAddr_) ? rs_ : &transit, buffer_, bufferSize_, logName_, &logFilter_);

	if (protocol.send(addr, id, data, datasize)) {
		++cntGood_;
		return true;
	}

	return false;
}

void DataXchg::enableLog(const std::string &name, const std::vector<uint8_t> *filter) {
	logName_ = name;
	logFilter_ = filter != NULL ? *filter : std::vector<uint8_t>();
}
