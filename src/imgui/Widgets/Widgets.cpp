#include "Widgets.h"
#include <ctime>

#define SLIDERS_COUNT (3 + 4 + 2 + 1)

Widgets* ImGuiWidgets;

Widgets::Widgets() {
    for (int i = 0; i < SLIDERS_COUNT; i++)
        sliderDragging.push_back(false);
}

void Widgets::CreateRoundedImage(ImTextureID id, float width, float height, float rounding) {
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    ImVec2 p_min = ImGui::GetCursorScreenPos();
    ImVec2 p_max = ImVec2(p_min.x + width, p_min.y + height);
    dl->AddImageRounded(id, p_min, p_max,
        ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(ImVec4(1, 1, 1, 1)),
        rounding);
}

std::pair<float, bool> Widgets::CustomSliderFloat(const char* label, float& value, float min, float max, const char* format, int index, float width) {
    ImGuiStyle& style = ImGui::GetStyle();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
    ImVec2 text_size = ImGui::CalcTextSize(label);
    char value_text[32];
    snprintf(value_text, 32, format, value);
    ImVec2 value_text_size = ImGui::CalcTextSize(value_text);
    ImVec2 slider_size = ImVec2(width, text_size.y - 6);
    ImVec2 slider_pos = ImVec2(cursor_pos.x, cursor_pos.y + text_size.y + style.ItemInnerSpacing.y);
    ImVec2 slider_end_pos = ImVec2(slider_pos.x + slider_size.x, slider_pos.y + slider_size.y);

    float fraction = (value - min) / (max - min);
    float handle_pos = slider_pos.x + fraction * slider_size.x;

    ImU32 color_background = ImGui::ColorConvertFloat4ToU32(ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
    ImU32 color_filled = ImGui::ColorConvertFloat4ToU32(ImVec4(0.2f, 0.4f, 0.8f, 1.0f));
    ImU32 color_slider = ImGui::ColorConvertFloat4ToU32(ImVec4(0.4f, 0.4f, 0.4f, 1.0f));

    draw_list->AddRectFilled(slider_pos, slider_end_pos, color_background, style.FrameRounding);

    ImVec2 filled_rect_end_pos = ImVec2(handle_pos, slider_end_pos.y);
    draw_list->AddRectFilled(slider_pos, filled_rect_end_pos, color_filled, style.FrameRounding);

    ImVec2 handle_size = ImVec2(13, slider_size.y);
    ImVec2 handle_rect_min = ImVec2(handle_pos - handle_size.x * 0.5f, slider_pos.y);
    ImVec2 handle_rect_max = ImVec2(handle_pos + handle_size.x * 0.5f, slider_end_pos.y - 1);

    // Отображение невидимой кнопки
    ImGui::SetCursorScreenPos(handle_rect_min);

    ImGui::PushID(index);
    ImGui::PushStyleColor(ImGuiCol_Button, color_slider);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color_slider);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, color_slider);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
    ImGui::Button("##slider_button", handle_size);
    ImGui::PopStyleVar(1);
    ImGui::PopStyleColor(3);
    ImGui::PopID();

    bool is_hovered = ImGui::IsMouseHoveringRect(handle_rect_min, handle_rect_max);
    if (is_hovered && ImGui::IsMouseDown(0))
    {
        bool flag = true;
        for (int i = 0; i < SLIDERS_COUNT; i++)
            if (i != index && sliderDragging[i] == true)
                flag = false;

        if (flag) {
            sliderDragging[index] = true;
        }
    }
    if (sliderDragging[index] && ImGui::IsMouseDown(0))
    {
        float mouse_x = ImGui::GetIO().MousePos.x;
        float new_fraction = (mouse_x - slider_pos.x) / slider_size.x;
        value = min + new_fraction * (max - min);
        value = std::clamp(value, min, max);
    }
    if (ImGui::IsMouseReleased(0))
    {
        sliderDragging[index] = false;
    }

    ImVec2 label_pos = ImVec2(slider_pos.x + (slider_size.x - text_size.x) * 0.5f, cursor_pos.y);
    ImGui::SetCursorScreenPos(label_pos);
    ImGui::Text(label);

    if (label != "")
        ImGui::SetCursorScreenPos(ImVec2(slider_end_pos.x + style.ItemInnerSpacing.x, cursor_pos.y + (text_size.y - value_text_size.y) * 0.5f));
    else
        ImGui::SetCursorScreenPos(ImVec2(slider_pos.x + ((slider_end_pos.x - slider_pos.x) / 2 - value_text_size.x / 2) + style.ItemInnerSpacing.x, cursor_pos.y + (text_size.y - value_text_size.y) * 0.5f));
    ImGui::Text(value_text);

    return std::pair{ value, sliderDragging[index] };
}

void Widgets::ShowHelpMarker(const char* id, const char* desc) {
    ImGui::PushID(id);
    ImGui::TextDisabled("(?)");
    ImGui::PopID();

    static std::unordered_map<std::string, float> alpha_map;
    static std::unordered_map<std::string, bool> hovered_map;
    const float fadeSpeed = 2.0f;
    std::string id_str(id);

    if (alpha_map.find(id_str) == alpha_map.end())
    {
        alpha_map[id_str] = 0.0f;
        hovered_map[id_str] = false;
    }

    if (ImGui::IsItemHovered())
    {
        hovered_map[id_str] = true;
        alpha_map[id_str] += ImGui::GetIO().DeltaTime * fadeSpeed;
        if (alpha_map[id_str] > 1.0f) alpha_map[id_str] = 1.0f;
    }
    else
    {
        hovered_map[id_str] = false;
        alpha_map[id_str] -= ImGui::GetIO().DeltaTime * fadeSpeed;
        if (alpha_map[id_str] < 0.0f) alpha_map[id_str] = 0.0f;
    }

    if (alpha_map[id_str] > 0.0f)
    {
        ImGui::SetNextWindowBgAlpha(alpha_map[id_str]);
        ImGui::BeginTooltip();
        ImVec4 textColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        textColor.w = alpha_map[id_str];

        ImGui::PushStyleColor(ImGuiCol_Text, textColor);

        ImGui::SetCursorPos(ImVec2(6.f, 4.f));

        ImGui::TextUnformatted(desc);

        ImGui::SameLine();
        ImGui::Dummy(ImVec2(6.f, 0.f));
        ImGui::Dummy(ImVec2(0.f, 4.f));

        ImGui::PopStyleColor();
        ImGui::EndTooltip();
    }
}

bool Widgets::LoadTextureFromFile(const char* filename, PDIRECT3DTEXTURE9* out_texture, LPDIRECT3DDEVICE9* g_pd3dDevice) {
    PDIRECT3DTEXTURE9 texture;
    HRESULT hr = D3DXCreateTextureFromFileA(*g_pd3dDevice, filename, &texture);
    if (hr != S_OK)
        return false;

    D3DSURFACE_DESC my_image_desc;
    texture->GetLevelDesc(0, &my_image_desc);
    *out_texture = texture;
    return true;
}

bool Widgets::CustomInputTextWithHint(const char* name, char* buffer, size_t bufferSize, const char* hint, float width, ImVec4 color, bool password) {
    if (password)
        ImGui::PushID(name);
    else
        ImGui::PushID(0);

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.18f, 0.27f, 0.0f));
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - width) / 2);
    ImGui::BeginGroup();
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PushItemWidth(width);
    ImGuiInputTextFlags flags = password ? ImGuiInputTextFlags_Password : ImGuiInputTextFlags_None;
    bool result = ImGui::InputTextWithHint(name, hint, buffer, bufferSize, flags);
    ImGui::PopItemWidth();
    ImGui::PopStyleColor();

    ImVec2 pos = ImGui::GetCursorScreenPos();
    auto drawList = ImGui::GetWindowDrawList();
    drawList->AddLine(ImVec2(pos.x, pos.y), ImVec2(pos.x + width, pos.y), IM_COL32(color.x * 255, color.y * 255, color.z * 255, color.w * 255), 3);

    ImGui::EndGroup();
    ImGui::PopID();

    return result;
}

bool Widgets::AnimButton(const char* label, const ImVec2& size, const std::vector<float>& duration) {
    static std::unordered_map<std::string, AnimButtonData> UI_ANIMBUT;

    ImVec4 defaultColor = ImGui::GetStyle().Colors[ImGuiCol_Button];
    ImVec4 hoveredColor = ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];
    ImVec4 activeColor = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];

    if (UI_ANIMBUT.find(label) == UI_ANIMBUT.end()) {
        UI_ANIMBUT[label] = {
            defaultColor,
            {0.0f, 0.0f},
            {false, false, 0.0f}
        };
    }
    AnimButtonData& pool = UI_ANIMBUT[label];

    if (pool.clicked[0] && pool.clicked[1]) {
        float currentTime = static_cast<float>(ImGui::GetTime());
        if (currentTime - pool.clicked[0] <= duration[1]) {
            pool.color = ImVec4(
                pool.color.x + (activeColor.x - pool.color.x) * (currentTime - pool.clicked[0]) / duration[1],
                pool.color.y + (activeColor.y - pool.color.y) * (currentTime - pool.clicked[0]) / duration[1],
                pool.color.z + (activeColor.z - pool.color.z) * (currentTime - pool.clicked[0]) / duration[1],
                pool.color.w + (activeColor.w - pool.color.w) * (currentTime - pool.clicked[0]) / duration[1]
            );
            goto no_hovered;
        }

        if (currentTime - pool.clicked[1] <= duration[1]) {
            ImVec4 targetColor = pool.hovered.cur ? hoveredColor : defaultColor;
            pool.color = ImVec4(
                pool.color.x + (targetColor.x - pool.color.x) * (currentTime - pool.clicked[1]) / duration[1],
                pool.color.y + (targetColor.y - pool.color.y) * (currentTime - pool.clicked[1]) / duration[1],
                pool.color.z + (targetColor.z - pool.color.z) * (currentTime - pool.clicked[1]) / duration[1],
                pool.color.w + (targetColor.w - pool.color.w) * (currentTime - pool.clicked[1]) / duration[1]
            );
            goto no_hovered;
        }
    }

    if (pool.hovered.clock != 0.0f) {
        float currentTime = static_cast<float>(ImGui::GetTime());
        if (currentTime - pool.hovered.clock <= duration[0]) {
            ImVec4 targetColor = pool.hovered.cur ? hoveredColor : defaultColor;
            pool.color = ImVec4(
                pool.color.x + (targetColor.x - pool.color.x) * (currentTime - pool.hovered.clock) / duration[0],
                pool.color.y + (targetColor.y - pool.color.y) * (currentTime - pool.hovered.clock) / duration[0],
                pool.color.z + (targetColor.z - pool.color.z) * (currentTime - pool.hovered.clock) / duration[0],
                pool.color.w + (targetColor.w - pool.color.w) * (currentTime - pool.hovered.clock) / duration[0]
            );
        }
        else {
            pool.color = pool.hovered.cur ? hoveredColor : defaultColor;
        }
    }

no_hovered:

    ImGui::PushStyleColor(ImGuiCol_Button, pool.color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pool.color);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, pool.color);
    bool result = ImGui::Button(label, size);
    ImGui::PopStyleColor(3);

    if (result) {
        float currentTime = static_cast<float>(ImGui::GetTime());
        pool.clicked = { currentTime, currentTime + duration[1] };
    }

    pool.hovered.cur = ImGui::IsItemHovered();
    if (pool.hovered.old != pool.hovered.cur) {
        pool.hovered.old = pool.hovered.cur;
        pool.hovered.clock = static_cast<float>(ImGui::GetTime());
    }

    return result;
}

void Widgets::ImGuiCenterTextByX(const char* text) {
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(text).x) / 2);
    ImGui::Text(text);
}

void Widgets::ImGuiCenterText(const char* text) {
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(text).x) / 2 - 2);
    ImGui::SetCursorPosY((ImGui::GetWindowSize().y - ImGui::CalcTextSize(text).y) / 2 - 2);
    ImGui::Text(text);
}

void Widgets::DistributeSquaresEvenly(std::vector<Square>& squares, ImVec2 startPos, ImVec2 areaSize, int rows, int cols)
{
    float cellWidth = areaSize.x / cols;
    float cellHeight = areaSize.y / rows;

    int index = 0;
    for (int row = 0; row < rows; ++row)
    {
        for (int col = 0; col < cols; ++col)
        {
            if (static_cast<size_t>(index) >= squares.size())
                return;

            Square& square = squares[index++];
            square.position = ImVec2(startPos.x + col * cellWidth + (cellWidth - square.size) / 2,
                startPos.y + row * cellHeight + (cellHeight - square.size) / 2);
            square.velocity = ImVec2((rand() % 5 - 2) * 0.25f, (rand() % 5 - 2) * 0.25f);
            square.size = 20.0f + rand() % 30;
        }
    }
}

void Widgets::RenderAnimatedSquares(ImVec2 startPos, ImVec2 areaSize)
{
    static std::vector<Square> squares;
    static bool initialized = false;

    if (!initialized)
    {
        squares.resize(12);

        srand(static_cast<unsigned>(time(nullptr)));
        DistributeSquaresEvenly(squares, startPos, areaSize, 4, 5);

        initialized = true;
    }

    ImVec2 mousePos = ImGui::GetMousePos();

    for (auto& square : squares)
    {
        square.position.x += square.velocity.x;
        square.position.y += square.velocity.y;

        if (square.position.x < startPos.x || square.position.x > startPos.x + areaSize.x - square.size)
            square.velocity.x *= -1;
        if (square.position.y < startPos.y || square.position.y > startPos.y + areaSize.y - square.size)
            square.velocity.y *= -1;
    }

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    const float detectionRadius = 40.0f;

    for (size_t i = 0; i < squares.size(); ++i)
    {
        const auto& square1 = squares[i];
        ImVec2 corners1[4] = {
            square1.position,
            ImVec2(square1.position.x + square1.size, square1.position.y),
            ImVec2(square1.position.x + square1.size, square1.position.y + square1.size),
            ImVec2(square1.position.x, square1.position.y + square1.size)
        };

        ImVec2 minPos1 = square1.position;
        ImVec2 maxPos1 = ImVec2(square1.position.x + square1.size, square1.position.y + square1.size);

        bool isHovered = mousePos.x >= minPos1.x && mousePos.x <= maxPos1.x &&
            mousePos.y >= minPos1.y && mousePos.y <= maxPos1.y;

        ImU32 fillColor = isHovered ? ImColor(135, 206, 250, 150) : ImColor(192, 192, 192, 15);
        ImU32 borderColor = ImColor(64, 64, 64, 255);

        draw_list->AddRectFilled(minPos1, maxPos1, fillColor);
        draw_list->AddRect(minPos1, maxPos1, borderColor);

        for (size_t j = i + 1; j < squares.size(); ++j)
        {
            const auto& square2 = squares[j];
            ImVec2 corners2[4] = {
                square2.position,
                ImVec2(square2.position.x + square2.size, square2.position.y),
                ImVec2(square2.position.x + square2.size, square2.position.y + square2.size),
                ImVec2(square2.position.x, square2.position.y + square2.size)
            };

            for (const auto& corner1 : corners1)
            {
                for (const auto& corner2 : corners2)
                {
                    float dist = sqrtf((corner1.x - corner2.x) * (corner1.x - corner2.x) +
                        (corner1.y - corner2.y) * (corner1.y - corner2.y));

                    if (dist < detectionRadius)
                    {
                        ImVec2 linePoints[3] = { corner1,
                                                 ImVec2((corner1.x + corner2.x) / 2, corner1.y),
                                                 corner2 };

                        draw_list->AddPolyline(linePoints, IM_ARRAYSIZE(linePoints),
                            ImColor(64, 64, 64, 200),
                            ImDrawFlags_None, 2.0f);
                    }
                }
            }
        }
    }
}

void Widgets::DrawGlowingText(const char* text, ImVec2 pos, ImVec4 textColor, ImVec4 glowColor, float glowSize)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 textSize = ImGui::CalcTextSize(text);

    for (float i = glowSize; i > 0; i -= 1.0f)
    {
        ImVec2 offset(i, i);

        draw_list->AddText(NULL, 0.0f, ImVec2(pos.x - offset.x, pos.y - offset.y), ImColor(glowColor), text);
        draw_list->AddText(NULL, 0.0f, ImVec2(pos.x + offset.x, pos.y - offset.y), ImColor(glowColor), text);
        draw_list->AddText(NULL, 0.0f, ImVec2(pos.x - offset.x, pos.y + offset.y), ImColor(glowColor), text);
        draw_list->AddText(NULL, 0.0f, ImVec2(pos.x + offset.x, pos.y + offset.y), ImColor(glowColor), text);
    }

    draw_list->AddText(NULL, 0.0f, pos, ImColor(textColor), text);
}