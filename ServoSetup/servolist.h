#ifndef SERVOLIST_H
#define SERVOLIST_H

#include <string>
#include <vector>
#include <map>

namespace pugi {
class xml_node;
}

class ServoList {
public:
	ServoList();
	~ServoList();

	struct Servo {
		Servo(const std::string &name_, int addr_, int group_, int transit_)
			: name(name_), addr(addr_), group(group_), transit(transit_) {}
		std::string name;
		int addr;
		int group;
		int transit;
	};

	void read(const std::string &filename);

	void getPresetNames(std::vector<std::string> &names) const;
	void getServoNames(const std::string &preset, std::vector<std::string> &names) const;

	size_t getNumOfServos(const std::string &preset) const;
	const Servo & getByIndex(const std::string &preset, size_t index) const;
	const Servo & getByName(const std::string &preset, const std::string &name) const;
	size_t getIndex(const std::string &preset, const std::string &name) const;

private:
	void readPreset(const pugi::xml_node &preset_node);

	typedef std::vector<Servo> servo_t;
	typedef std::map<std::string, servo_t> preset_t;

	preset_t preset_;

	const Servo empty_;
};

#endif
