#ifndef WIDGETSIMGUI_FILE_H
#define WIDGETSIMGUI_FILE_H

#include <unordered_map>
#include <d3d9.h>
#include <algorithm>

#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include "DirectX/d3dx9.h"

class Widgets {
public:
    Widgets();

    struct Square {
        ImVec2 position;
        ImVec2 velocity;
        ImVec4 color;
        float size;
    };

    bool CustomInputTextWithHint(const char* name, char* buffer, size_t bufferSize, const char* hint, float width, ImVec4 color, bool password);
    bool AnimButton(const char* label, const ImVec2& size, const std::vector<float>& duration);
    bool LoadTextureFromFile(const char* filename, PDIRECT3DTEXTURE9* out_texture, LPDIRECT3DDEVICE9* g_pd3dDevice);
    void CreateRoundedImage(ImTextureID id, float width, float height, float rounding);
    void ShowHelpMarker(const char* id, const char* desc);
    void DrawGlowingText(const char* text, ImVec2 pos, ImVec4 textColor, ImVec4 glowColor, float glowSize);
    void RenderAnimatedSquares(ImVec2 startPos, ImVec2 areaSize);
    void DistributeSquaresEvenly(std::vector<Square>& squares, ImVec2 startPos, ImVec2 areaSize, int rows, int cols);
    std::pair<float, bool> CustomSliderFloat(const char* label, float& value, float min, float max, const char* format, int index, float width);

    void ImGuiCenterText(const char* text);
    void ImGuiCenterTextByX(const char* text);

private:
    struct AnimButtonData {
        ImVec4 color;
        std::vector<float> clicked;
        struct {
            bool cur;
            bool old;
            float clock;
        } hovered;
    };

    std::vector<bool> sliderDragging;
};

extern Widgets* ImGuiWidgets;

#endif