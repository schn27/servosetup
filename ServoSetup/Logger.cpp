#include "stdafx.h"
#include "Logger.h"

Logger::Logger() {
}

Logger::~Logger() {
}

void Logger::open() {
	out_.open("d:\\servo.csv", std::ios_base::trunc);

	if (out_.is_open()) {
		out_ << "Pos,Spd,Uin,Iout" << std::endl;
	}
}

void Logger::close() {
	if (out_.is_open()) {
		out_.close();
	}
}

void Logger::write(int position, int speed, int uin, int iout) {
	if (out_.is_open()) {
		out_ << position << "," << speed << "," << uin << "," << iout << std::endl;
	}
}
