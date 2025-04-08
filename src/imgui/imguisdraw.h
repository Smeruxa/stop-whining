#ifndef IMGUISDRAW_FILE_H
#define IMGUISDRAW_FILE_H

#pragma execution_character_set("utf-8")

#include "main.h"
#include "config/config.h"
#include "FaAwesome/IconsFontAwesome6.h"

#include "Widgets/Widgets.h"
#include "HotKey/ImHotKey.h"

enum PLUGIN_STATES
{
	CROSSHAIR,
	PLAYER,
	VISUAL,
	ADDITIONAL,
	VEHICLE,
	DESTROY
};

class imguiDraw {
public:
	imguiDraw();
	void Draw();

private:
	PLUGIN_STATES CURRENT_STATE = PLUGIN_STATES::CROSSHAIR;

	void RenderButtons(std::vector<std::pair<PLUGIN_STATES, std::vector<std::string>>> list, PLUGIN_STATES& currentState, float width);
};

extern imguiDraw* gImguiDraw;

#endif