#ifndef DATAXCHG_H
#define DATAXCHG_H

#include <stdint.h>
#include <string>
#include <vector>
#include "afxwin.h"
#include "afxcmn.h"
#include "afxmt.h"
#include "Thread.h"
#include "../SerialPort/serialport.h"

class SerialPort;

class DataXchg : public Thread {
public:
	DataXchg(SerialPort *rs, uint8_t addr, int transitAddr, bool broadcast = false);
	virtual ~DataXchg();

	void setAddr(uint8_t addr);
	void setPosition(int value);
	
	const std::string & getInfo() const {
		return info_;
	}

	void getStatus(float &position, float &speed, float &uin, float &iout);
	void reqParam(size_t index);
	bool getParam(size_t index, int16_t &value);
	void setParam(size_t index, int16_t value);
	void setAddrCfg(uint8_t addr, uint8_t addrAlias);
	void manualCfg(uint8_t cmd);

	int getCntGood() const {
		return cntGood_;
	}

	int getCntBad() const {
		return cntBad_;
	}

private:
	virtual int threadProc(void *p);

	void updateInfo();
	void update();
	void updateParams(bool all);
	void doPortOpen();
	void doPortClose();
	void doWriteAddr();
	void doManual(uint8_t cmd);

	bool writeParam(uint8_t id, int16_t value);
	bool readParam(uint8_t id, int16_t &value);

	void processData();
	bool request(uint8_t addr, uint8_t id, uint8_t *data, uint8_t dataSize, uint8_t *response, uint8_t &responseSize);
	bool requestNoAnswer(uint8_t addr, uint8_t id, uint8_t *data, uint8_t dataSize);

	enum {
		msgWriteAddr = msgBase,
		msgSetAddr,
		msgManual
	};

	CCriticalSection criticalSection_;

	SerialPort *rs_;
	
	static const int bufferSize_;
	uint8_t *buffer_;

	static const int buffer2Size_;
	uint8_t *buffer2_;

	uint8_t addr_;
	int transitAddr_;

	int cntGood_;
	int cntBad_;

	bool cfgread_;
	
	enum {eParIdle = 0, eParReadReq, eParReadResp, eParWriteReq};
	
	struct param_t {
		param_t() : value(0), state(eParIdle) {}
		int16_t value;
		uint8_t state;
	};

	std::vector<param_t> params_;

	int cmdPosition_;
	float position_;
	float speed_;
	float uin_;
	float iout_;
	std::string info_;
	bool noInfo_;

	uint8_t cfgAddr_;
	uint8_t cfgAddrAlias_;

	bool manualActive_;
	bool broadcast_;
};

#endif
