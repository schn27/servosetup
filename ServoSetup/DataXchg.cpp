#include "protocol.h"
#include "protocoladapter.h"
#include "DataXchg.h"

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
#if !defined(OGOROD_SERVO)
const float PositionScale = 1.0f;
const float SpeedScale = 1.0f;
const float UinScale = 0.001f;
const float IoutScale = 0.001f;
#else
const float PositionScale = 90.0f / 1500.0f;
const float SpeedScale = 3;
const float UinScale = 0.012f;
const float IoutScale = 0.001f;
#endif
}

}

const int DataXchg::bufferSize_ = 256;
const int DataXchg::buffer2Size_ = 256 + 4;

DataXchg::DataXchg(SerialPort *rs, uint8_t addr, int transitAddr, bool broadcast)
		: thread_(nullptr)
		, stop_(false)
		, rs_(rs)
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
		, writeAddrReq_(false)
		, manualCmd_(0)
		, manualCmdReq_(false)
		, manualActive_(false)
		, broadcast_(broadcast)
		, logName_("")
		, logFilter_() {
	params_.reserve(256);

	for (int i = 0; i < 256; ++i) {
		params_.push_back(param_t());
	}

	thread_ = new std::thread(&DataXchg::run, this);
}

DataXchg::~DataXchg() {
	stop_ = true;
	thread_->join();
	delete thread_;

	delete [] buffer_;
	delete [] buffer2_;
}

void DataXchg::getStatus(float &position, float &speed, float &uin, float &iout) {
	std::lock_guard<std::mutex> _(lock_);
	position = position_;
	speed = speed_;
	uin = uin_;
	iout = iout_;
}

void DataXchg::setPosition(int value) {
	std::lock_guard<std::mutex> _(lock_);
	cmdPosition_ = value;
}

void DataXchg::reqParam(size_t index) {
	if (index >= params_.size()) {
		return;
	}

	std::lock_guard<std::mutex> _(lock_);
	params_[index].state = eParReadReq;
}

bool DataXchg::getParam(size_t index, int16_t &value) {
	if (index >= params_.size()) {
		return false;
	}

	bool res = false;

	std::lock_guard<std::mutex> _(lock_);

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

	std::lock_guard<std::mutex> _(lock_);
	param_t &param = params_[index];
	param.value = value;
	param.state = eParWriteReq;
}

void DataXchg::setAddrCfg(uint8_t addr, uint8_t addrAlias) {
	cfgAddr_ = addr;
	cfgAddrAlias_ = addrAlias;
	writeAddrReq_ = true;
}

void DataXchg::setAddr(uint8_t addr) {
	addr_ = addr;
	noInfo_ = true;
}

void DataXchg::manualCfg(uint8_t cmd) {
	manualCmd_ = cmd;
	manualCmdReq_ = true;
}

void DataXchg::run() {
	while (!stop_) {
		if (writeAddrReq_) {
			doWriteAddr();
			writeAddrReq_ = false;
		}
		
		if (manualCmdReq_) {
			doManual();
			manualCmdReq_ = false;
		}

		updateInfo();
		update();
		updateParams(true);
	}
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
		std::lock_guard<std::mutex> _(lock_);
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
		request(addr_, idSetPos, buf, 2, nullptr, n);

		if (transitAddr_ >= 0) {
			request(transitAddr_, idSetPos, buf, 3, nullptr, n);
		}
	}

	uint8_t bufsize = 8;
	if (request(addr_, idGetState, buf, 0, buf, bufsize)) {
		std::lock_guard<std::mutex> _(lock_);
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
			std::lock_guard<std::mutex> _(lock_);
			param = params_[i];
		}

		if (param.state == eParReadReq)	{
			if (readParam(i, param.value)) {
				param.state = eParReadResp;
	
				std::lock_guard<std::mutex> _(lock_);
				params_[i] = param;
			}

			updated = true;
		} else if (param.state == eParWriteReq) {
			if (writeParam(i, param.value)) {
				param.state = eParIdle;

				std::lock_guard<std::mutex> _(lock_);
				params_[i] = param;
			}

			updated = true;
		}
	}
}

bool DataXchg::writeParam(uint8_t id, int16_t value) {
	for (int i = 0; i < 3; ++i) {
		uint8_t buf[3] = {id, uint8_t(value & 0xFF), uint8_t(value >> 8)};
		uint8_t n = 0;
		if (request(addr_, idSetPar, buf, 3, nullptr, n)) {
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
		uint8_t buf[6] = {uint8_t(rate), uint8_t(rate >> 8), uint8_t(rate >> 16), uint8_t(rate >> 24), cfgAddr_, cfgAddrAlias_};
		uint8_t n = 0;
		if (request(addr_, idSetRS485, buf, 6, nullptr, n)) {
			return;
		}
	}
}

void DataXchg::doManual() {
	if (broadcast_) {
		return;
	}

	if (manualCmd_ == 0) {
		manualActive_ = true;
	} else if (manualCmd_ == 4) {
		manualActive_ = false;
	}

	for (int i = 0; i < 3; ++i) {
		uint8_t n = 0;
		if (request(addr_, idManualSetup, &manualCmd_, 1, nullptr, n)) {
			return;
		}
	}
}

// отправка запроса и приём ответа
bool DataXchg::request(uint8_t addr, uint8_t id, uint8_t *data, uint8_t datasize, uint8_t *response, uint8_t &responsesize) {
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

	if (response != nullptr) {
		memcpy(response, protocol.getDataPointer(), responsesize);
	}

	++cntGood_;

	return true;
}

// отправка запроса без ожидания ответа
bool DataXchg::requestNoAnswer(uint8_t addr, uint8_t id, uint8_t *data, uint8_t datasize) {
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
	logFilter_ = filter != nullptr ? *filter : std::vector<uint8_t>();
}
