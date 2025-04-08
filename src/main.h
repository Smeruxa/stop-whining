#ifndef MAIN_FILE_H
#define MAIN_FILE_H

#include <vector>
#include <imgui.h>
#include <windows.h>

#include "ktcoro/ktcoro_wait.hpp"

struct vars {
    bool is_open_menu = false;
    bool temp_is_open_menu = false;

    std::vector<ImFont*> fonts;
    std::vector<ImFont*> faFonts;

    float displaySize[2] = { 0.f, 0.f };
    float circlePos[2] = { 0.f, 0.f };

    ktcoro_tasklist tasklist;

    DWORD s_samp;
}; extern vars variables;

#endif