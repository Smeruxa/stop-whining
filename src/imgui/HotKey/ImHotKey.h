#ifndef IMGUIHOTKEY_FILE_H
#define IMGUIHOTKEY_FILE_H

#include <imgui.h>

class ImHotKey {
public:
    bool Hotkey(const char* label, ImGuiKey& key, ImVec2 size);
private:
    ImGuiID g_last_key_label_id = {};
};

extern ImHotKey* gHotKey;

#endif