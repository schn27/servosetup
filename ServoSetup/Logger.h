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
	void write(int position, int speed, int uin, int iout);

private:
	std::ofstream out_;
};

#endif
