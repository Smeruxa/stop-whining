#include "ImHotKey.h"

#include <string>

ImHotKey* gHotKey;

bool ImHotKey::Hotkey(const char* label, ImGuiKey& key, ImVec2 size)
{
	const ImGuiID label_id = ImGui::GetID(label);

	bool can_set_key = g_last_key_label_id == label_id;

	std::string_view key_name = {};

	if (can_set_key)
		key_name = "...";
	else
		key_name = ImGui::GetKeyName(key);

	ImGui::PushID(label_id);

	if (ImGui::Button(key_name.data(), ImVec2(size.x, size.y)))
	{
		can_set_key = true;
		g_last_key_label_id = label_id;
	}

	ImGui::PopID();

	if (can_set_key)
	{
		for (ImGuiKey k = ImGuiKey_KeysData_OFFSET; k < ImGuiKey_COUNT; k = (ImGuiKey)(k + 1))
		{
			if (!ImGui::IsKeyDown(k))
				continue;

			key = k;
			g_last_key_label_id = 0;
			break;
		}
	}

	return can_set_key && g_last_key_label_id == 0;
}