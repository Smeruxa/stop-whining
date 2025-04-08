#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H

#include "json/json.hpp"
#include "imgui.h"

class config {
public:
	config(const char* filename);

	bool save();
	nlohmann::json data;
private:
	const char* filename;
};

extern config* gConfig;

#endif