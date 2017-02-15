#ifndef COMM_H
#define COMM_H

#include <stdint.h>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include "../SerialPort/serialport.h"

class SerialPort;

class Comm {
public:
	Comm(SerialPort *rs, uint8_t addr, int transitAddr, bool broadcast = false);
	virtual ~Comm();

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

	size_t getCntGood() const {
		return cntGood_;
	}

	size_t getCntBad() const {
		return cntBad_;
	}

	void enableLog(const std::string &name, const std::vector<uint8_t> *filter = nullptr);

private:
	virtual void run();

	void updateInfo();
	void update();
	void updateParams(bool all);
	void doWriteAddr();
	void doManual();

	bool writeParam(size_t id, int16_t value);
	bool readParam(size_t id, int16_t &value);

	bool request(uint8_t addr, uint8_t id, uint8_t *data, size_t dataSize, uint8_t *response, size_t &responseSize);
	bool requestNoAnswer(uint8_t addr, uint8_t id, uint8_t *data, size_t dataSize);

	std::thread *thread_;
	volatile bool stop_;
	std::mutex lock_;

	SerialPort *rs_;
	
	static const size_t bufferSize_;
	uint8_t *buffer_;

	static const size_t buffer2Size_;
	uint8_t *buffer2_;

	uint8_t addr_;
	int transitAddr_;

	size_t cntGood_;
	size_t cntBad_;

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
	volatile bool writeAddrReq_;

	uint8_t manualCmd_;
	volatile bool manualCmdReq_;

	bool manualActive_;
	bool broadcast_;

	std::string logName_;
	std::vector<uint8_t> logFilter_;
};

#endif
