#include <sstream>
#include <string>
#include "../pugixml/pugixml.hpp"
#include "paramconfig.h"

using namespace pugi;

ParamConfig::ParamConfig() {
}

ParamConfig::~ParamConfig() {
}

void ParamConfig::read(const std::string &filename) {
	xml_document doc;

	if (!doc.load_file(filename.c_str())) {
		return;
	}

	xml_node node = doc.first_element_by_path("config/params/param");

	params_.clear();

	while (!node.empty()) {
		int id = node.attribute("id").as_int();
		std::string name = node.attribute("name").value();
		int fixedPoint = node.attribute("fixedpoint").as_int();

		xml_attribute color_attr = node.attribute("color");
		
		int color = 0xFFFFFF;
		if (!color_attr.empty()) {
			std::stringstream ss;
			ss << std::hex << color_attr.value();
			ss >> color;
		}

		params_.push_back(Param(id, name, color, fixedPoint));

		node = node.next_sibling();
	}
}

int ParamConfig::getNumOfParams() const {
	return params_.size();
}

const ParamConfig::Param & ParamConfig::getParam(int index) const {
	return params_[index];
}

int ParamConfig::getIndex(const std::string &name) const {
	for (int i = 0, n = params_.size(); i < n; ++i) {
		if (params_[i].name == name) {
			return i;
		}
	}
	
	return -1;
}

