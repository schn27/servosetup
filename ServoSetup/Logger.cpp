#include "Logger.h"

Logger::Logger(const std::string &filename) {
	out_.open(filename, std::ios_base::trunc);

	if (out_.is_open()) {
		out_ << "Pos,Spd,Uin,Iout" << std::endl;
	}
}

Logger::~Logger() {
	if (out_.is_open()) {
		out_.close();
	}
}

void Logger::write(float position, float speed, float uin, float iout) {
	if (out_.is_open()) {
		out_ << position << "," << speed << "," << uin << "," << iout << std::endl;
	}
}
