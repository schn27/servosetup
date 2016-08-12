#include <sstream>
#include <string>
#include "../pugixml/pugixml.hpp"
#include "servolist.h"

using namespace pugi;

ServoList::ServoList()
		: empty_("?", 0, 0, -1) {
}

ServoList::~ServoList() {
}

void ServoList::read(const std::string &filename) {
	xml_document doc;

	if (!doc.load_file(filename.c_str())) {
		return;
	}

	preset_.clear();

	for (xml_node node = doc.first_element_by_path("config/preset"); !node.empty(); node = node.next_sibling()) {
		readPreset(node);
	}
}


void ServoList::readPreset(const pugi::xml_node &preset_node) {
	std::string preset_name = preset_node.attribute("name").value();

	if (preset_name.empty()) {
		preset_name = "Default";
	}

	servo_t &servo = preset_[preset_name];

	for (xml_node node = preset_node.first_element_by_path("servo"); !node.empty(); node = node.next_sibling()) {
		int group = node.attribute("group").as_int();
		int addr = node.attribute("addr").as_int();
		std::string name = node.attribute("name").value();
		
		xml_attribute transit_attr = node.attribute("transit");
		int transit = transit_attr.empty() ? -1 : transit_attr.as_int();

		servo.push_back(Servo(name, addr, group, transit));
	}
}


void ServoList::getPresetNames(std::vector<std::string> &names) const {
	names.clear();

	for (preset_t::const_iterator p = preset_.begin(), end = preset_.end(); p != end; ++p) {
		names.push_back(p->first);
	}
}


void ServoList::getServoNames(const std::string &preset, std::vector<std::string> &names) const {
	names.clear();

	preset_t::const_iterator p = preset_.find(preset);

	if (p != preset_.end()) {
		const servo_t &servo = p->second;
		for (servo_t::const_iterator s = servo.begin(), end = servo.end(); s != end; ++s)
			names.push_back(s->name);
	}
}


size_t ServoList::getNumOfServos(const std::string &preset) const {
	preset_t::const_iterator p = preset_.find(preset);

	if (p != preset_.end()) {
		const servo_t &servo = p->second;
		return servo.size();
	}

	return 0;
}

const ServoList::Servo & ServoList::getByIndex(const std::string &preset, size_t index) const {
	preset_t::const_iterator p = preset_.find(preset);

	if (p != preset_.end()) {
		const servo_t &servo = p->second;
		return (index < servo.size()) ? servo[index] : empty_;
	}

	return empty_;
}

const ServoList::Servo & ServoList::getByName(const std::string &preset, const std::string &name) const {
	return getByIndex(preset, getIndex(preset, name));
}

size_t ServoList::getIndex(const std::string &preset, const std::string &name) const {
	preset_t::const_iterator p = preset_.find(preset);

	if (p != preset_.end()) {
		const servo_t &servo = p->second;
		for (int i = 0, n = servo.size(); i < n; ++i) {
			if (servo[i].name == name) {
				return i;
			}
		}
	}
	
	return ~0UL;
}

