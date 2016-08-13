#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>

class Logger {
public:
	Logger(const std::string &filename);
	~Logger();

	void write(float position, float speed, float uin, float iout);

private:
	std::ofstream out_;
};

#endif
