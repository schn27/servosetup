#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>
#include <iostream>
#include <fstream>
#include <iomanip>

class Logger {
public:
	Logger();
	~Logger();

	void open();
	void close();
	void write(float position, float speed, float uin, float iout);

private:
	std::ofstream out_;
};

#endif
