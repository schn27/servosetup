#include "stdafx.h"
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include "../pugixml/pugixml.hpp"
#include "../pugixml/pugiconfig.hpp"
#include "../SerialPort/Rs232.h"
#include "../SerialPort/UdpTunnel.h"
#include "../SerialPort/nullserialport.h"
#include "connection.h"

using namespace pugi;

Connection::Connection(const std::string &filename)
		: filename_(filename) {
}

Connection::~Connection() {
}

SerialPort *Connection::getIf() {
	xml_document doc;

	if (!doc.load_file(filename_.c_str())) {
		return new NullSerialPort();
	}

	return CreateInterface(doc.child("config").child("connect"));
}

SerialPort *Connection::CreateInterface(xml_node &node) {
	std::string type(node.attribute("interface").value());

	xml_node iface = node.child(type.c_str());

	if (iface.empty()) {
		return new NullSerialPort();
	} else if (type == "serial") {
		return new Rs232(
			iface.attribute("port").value(), 
			iface.attribute("baudrate").as_int(),
			100);	// timeout
	} else if (type == "udp") {
		return new UdpTunnel(
			iface.attribute("self").value(), iface.attribute("port-in").as_int(),
			iface.attribute("addr").value(), iface.attribute("port-out").as_int());
	}

	return NULL;
}
