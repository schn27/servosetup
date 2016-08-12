#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>

class SerialPort;

namespace pugi {
class xml_node;
}

class Connection {
public:
	Connection(const std::string &filename);
	~Connection();

	SerialPort *getIf();

private:
	SerialPort *CreateInterface(pugi::xml_node &node);

	std::string filename_;
};


#endif
