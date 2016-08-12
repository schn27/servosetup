#ifndef PARAMCONFIG_H
#define PARAMCONFIG_H

#include <string>
#include <vector>

class ParamConfig {
public:
	ParamConfig();
	~ParamConfig();

	struct Param {
		Param(int id_, const std::string &name_, int color_, int fixedPoint_)
			: id(id_), name(name_), color(color_), fixedPoint(fixedPoint_)
		{}
		int id;
		std::string name;
		int color;
		int fixedPoint;
	};

	void read(const std::string &filename);
	int getNumOfParams() const;
	const Param & getParam(int index) const;
	int getIndex(const std::string &name) const;

private:
	typedef std::vector<Param> params_t;
	params_t params_;
};

#endif
