#include "d3dhook.h"
#include "plugin/cheats/utils/lpUtils.h"
#include "config/config.h"
#include "sampver/sampver.h"
#include "plugin/cheats/utils/lpUtils.h"

#include "fonts_compressed/fasolid.h"
#include "fonts_compressed/roboto.h"
#include "fonts_compressed/tahoma.h"

#include <vector>
#include <ePedBones.h>
#include <CPools.h>
#include <CCamera.h>
#include <sampapi/CNetGame.h>
#include <sampapi/CGame.h>

HWND game_hwnd = [] {
    if (const HWND *hwnd_ptr = *reinterpret_cast<HWND **>(0xC17054); hwnd_ptr != nullptr) {
        return *hwnd_ptr;
    }
    game_instance_init_hook.after += [](const auto &hook, const HWND &return_value, HINSTANCE inst) {
        game_hwnd = return_value;
    };
    return static_cast<HWND>(nullptr);
}();

std::array<uint8_t, 3> convertColorToRGB(sampapi::D3DCOLOR color);
void lines(int id);
void boxes(int id);
void skeleton(int id);
void nametags(int id);
void chams(int id, IDirect3DDevice9* device_ptr);

std::optional<HRESULT> d3dhook::on_present(const decltype(present_hook) &hook, IDirect3DDevice9 *device_ptr, const RECT *, const RECT *, HWND wnd, const RGNDATA *) {
    static bool ImGui_inited = false;
    if (!ImGui_inited) {
        ImGui::CreateContext();
        const ImGuiIO &io = ImGui::GetIO();
        (void) io;
        ImGui_ImplWin32_Init(game_hwnd);
        ImGui_ImplDX9_Init(device_ptr);
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        ImGui::GetIO().IniFilename = NULL;

        ImFontConfig font_config;
        font_config.OversampleH = 1; //or 2 is the same
        font_config.OversampleV = 1;
        font_config.PixelSnapH = 1;

        static const ImWchar ranges[] =
        {
            0x0020, 0x00FF, // Basic Latin + Latin Supplement
            0x0400, 0x044F, // Cyrillic
            0,
        };

        float iSizeFont[4] = { 20.f, 28.f, 40.f, 17.f };

        for (int i = 0; i < 4; i++)
            variables.fonts.push_back(io.Fonts->AddFontFromMemoryCompressedTTF(Roboto_compressed_data, Roboto_compressed_size, iSizeFont[i], &font_config, ranges));

        ImFontGlyphRangesBuilder builder;
        builder.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
        builder.AddText("‚„…†‡ˆ‰‹‘’“”•–—™›¹");

        ImVector<ImWchar> defaultGlyphRanges;
        builder.BuildRanges(&defaultGlyphRanges);

        variables.fonts.push_back(ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(Tahoma_compressed_data, Tahoma_compressed_size, 14, nullptr, defaultGlyphRanges.Data));

        ImFontConfig config;
        static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

        float iSizeLetters[2] = { 18.f, 2.f };

        for (int i = 0; i < 2; i++) {
            config.GlyphMinAdvanceX = iSizeLetters[i];
            variables.faFonts.push_back(io.Fonts->AddFontFromMemoryCompressedTTF(FaSolid_compressed_data, FaSolid_compressed_size, iSizeLetters[i], &config, icon_ranges));
        }

        changeStyleSheet();

        const auto latest_wndproc_ptr = GetWindowLongPtrA(game_hwnd, GWLP_WNDPROC);
        wndproc_hook.set_dest(latest_wndproc_ptr);
        wndproc_hook.set_cb(&keyhook::on_wndproc);
        wndproc_hook.install();
        ImGui_inited = true;
    }

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // ADDITIONAL

    utils->setCrosshairState(!gConfig->data["additional"]["crosshair"]["state"].get<bool>());
    if (gConfig->data["additional"]["crosshair"]["state"].get<bool>()) {
        int style = gConfig->data["additional"]["crosshair"]["style"].get<int>();
        if (style == 0 || (ImGui::IsKeyDown(ImGuiKey_MouseRight) && style == 1)) {
            ImDrawList* drawlist = ImGui::GetBackgroundDrawList();
            CVector crosshairPos;
            utils->getCrosshairPosition(&crosshairPos);
            CVector convertedPos;
            utils->CalcScreenCoords(&crosshairPos, &convertedPos);

            drawlist->AddLine(ImVec2(convertedPos.x, convertedPos.y + 10), ImVec2(convertedPos.x, convertedPos.y - 10), 0xFF0000FF, 1.f);
            drawlist->AddLine(ImVec2(convertedPos.x + 10, convertedPos.y), ImVec2(convertedPos.x - 10, convertedPos.y), 0xFF0000FF, 1.f);
        }
    }

    // VISUAL

    for (int id = 0; id < 1004; id++) {
        if (gSampVersion->getSampVersion() == samp_ver::v037r31) {
            if (!sampapi::v037r3::RefNetGame()->GetPlayerPool()->IsConnected(id))
                continue;

            sampapi::v037r3::CPed* ped = sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(id)->m_pPed;
            if (!ped || ped == nullptr)
                continue;

            CPed* pPed = CPools::GetPed(ped->m_handle);
            if (!pPed || pPed == nullptr)
                continue;

            if (!pPed->GetIsOnScreen())
                continue;

            if (sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(id)->GetDistanceToLocalPlayer() > gConfig->data["visual"]["distance"].get<float>())
                continue;

            sampapi::CVector playerPosition = sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(id)->m_onfootData.m_position, convertedPlayerPosition;
            utils->CalcScreenCoords(&playerPosition, &convertedPlayerPosition);
        }
        else {
            if (!sampapi::v037r1::RefNetGame()->GetPlayerPool()->IsConnected(id))
                continue;

            sampapi::v037r1::CPed* ped = sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetPlayer(id)->m_pPed;
            if (!ped || ped == nullptr)
                continue;

            CPed* pPed = CPools::GetPed(ped->m_handle);
            if (!pPed || pPed == nullptr)
                continue;

            if (!pPed->GetIsOnScreen())
                continue;

            if (sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetPlayer(id)->GetDistanceToLocalPlayer() > gConfig->data["visual"]["distance"].get<float>())
                continue;

            sampapi::CVector playerPosition = sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetPlayer(id)->m_onfootData.m_position, convertedPlayerPosition;
            utils->CalcScreenCoords(&playerPosition, &convertedPlayerPosition);
        }

        // functions call

        if (gSampVersion->getSampVersion() == samp_ver::v037r31)
            sampapi::v037r3::RefNetGame()->m_pSettings->m_bNameTags = gConfig->data["visual"]["nametags"]["defaultNametags"].get<bool>();
        else
            sampapi::v037r1::RefNetGame()->m_pSettings->m_bNameTags = gConfig->data["visual"]["nametags"]["defaultNametags"].get<bool>();

        if (gConfig->data["visual"]["boxes"]["state"].get<bool>())
            boxes(id);
        if (gConfig->data["visual"]["lines"]["state"].get<bool>())
            lines(id);
        if (gConfig->data["visual"]["skeleton"]["state"].get<bool>())
            skeleton(id);
        if (gConfig->data["visual"]["nametags"]["state"].get<bool>())
            nametags(id);
        if (gConfig->data["visual"]["chams"]["state"].get<bool>())
            chams(id, device_ptr);
    }

    variables.displaySize[0] = ImGui::GetIO().DisplaySize.x;
    variables.displaySize[1] = ImGui::GetIO().DisplaySize.y;

    if (gConfig->data["crosshair"]["FOV"]["show"].get<bool>())
        ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(variables.circlePos[0], variables.circlePos[1]), gConfig->data["crosshair"]["FOV"]["radius"].get<float>(), ImGui::GetColorU32(ImVec4(0.400f, 0.988f, 0.945f, 0.35f)), 50);

    if (variables.is_open_menu) {
        gImguiDraw->Draw();
    }
    if (variables.temp_is_open_menu != variables.is_open_menu) {
        variables.temp_is_open_menu = variables.is_open_menu;
        showCursor(variables.is_open_menu);
    }
    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    return std::nullopt;
}

void outlineText(ImDrawList* drawList, float x, float y, const char* text, ImU32 textColor, ImU32 outlineColor, float outlineThickness = 1.0f) {
    ImVec2 offsets[4] = {
        { -outlineThickness, -outlineThickness },
        {  outlineThickness, -outlineThickness },
        { -outlineThickness,  outlineThickness },
        {  outlineThickness,  outlineThickness }
    };

    for (const auto& offset : offsets) {
        drawList->AddText(ImVec2(x + offset.x, y + offset.y), outlineColor, text);
    }

    drawList->AddText(ImVec2(x, y), textColor, text);
}

uint32_t interpolateHealth(float health) {
    health = std::max(0.0f, std::min(100.0f, health));
    float t = health / 100.0f;

    int r1 = 255, g1 = 0, b1 = 0;
    int r2 = 255, g2 = 165, b2 = 0;
    int r3 = 0, g3 = 255, b3 = 0;

    int r = t <= 0.5f ? static_cast<int>(std::floor(r1 * (1 - t * 2) + r2 * t * 2))
        : static_cast<int>(std::floor(r2 * (1 - (t - 0.5f) * 2) + r3 * (t - 0.5f) * 2));
    int g = t <= 0.5f ? static_cast<int>(std::floor(g1 * (1 - t * 2) + g2 * t * 2))
        : static_cast<int>(std::floor(g2 * (1 - (t - 0.5f) * 2) + g3 * (t - 0.5f) * 2));
    int b = t <= 0.5f ? static_cast<int>(std::floor(b1 * (1 - t * 2) + b2 * t * 2))
        : static_cast<int>(std::floor(b2 * (1 - (t - 0.5f) * 2) + b3 * (t - 0.5f) * 2));

    return (255 << 24) | (r << 16) | (g << 8) | b;
}

uint32_t convert(uint32_t argbColor) {
    return (argbColor & 0xFF00FF00) | ((argbColor & 0xFF) << 16) | ((argbColor >> 16) & 0xFF);
}

std::pair<std::string, ImU32> classifyDistance(float distance) {
    if (distance < 35) {
        return { "D", convert(0xFFFF0000) };
    }
    else if (distance >= 35 && distance < 90) {
        return { "M", convert(0xFF0000FF) };
    }
    else if (distance >= 90 && distance <= 100) {
        return { "R", convert(0xFF00FF00) };
    }
    else {
        return { "", convert(0xFFFFFFFF) };
    }
}

void nametags(int id) {
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImGui::PushFont(variables.fonts[4]);
    if (gSampVersion->getSampVersion() == samp_ver::v037r1) {
        sampapi::CVector headPos;
        sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetPlayer(id)->m_pPed->GetBonePosition(8, &headPos);
        CVector camPosition = TheCamera.GetPosition();

        float dist = utils->getDistanceBetweenCoords3d(headPos.x, headPos.y, headPos.z, camPosition.x, camPosition.y, camPosition.z);

        if (dist <= gConfig->data["visual"]["distance"].get<float>()) {
            ImVec2 wpos;
            CVector convertedWPos;
            CVector headPosIncludeDistance = CVector(headPos.x, headPos.y, headPos.z + 0.2f + (dist * 0.025f));
            utils->CalcScreenCoords(&headPosIncludeDistance, &convertedWPos);
            wpos = ImVec2(convertedWPos.x, convertedWPos.y);

            float textHeight = ImGui::GetTextLineHeight();
            int spacing = 2;

            std::vector<std::pair<std::string, ImU32>> segments = {
                {sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetName(id), convert(sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetPlayer(id)->GetColorAsARGB())},
                {"(" + std::to_string(id) + ")", 0xFFFFFFFF},
                {sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetPlayer(id)->m_nStatus == 2 ? "A" : "", convert(0xFF00FF00)},
                classifyDistance(dist)
            };

            float totalWidth = 0.0f;
            std::vector<float> segmentWidths;
            for (const auto& segment : segments) {
                float width = ImGui::CalcTextSize(segment.first.c_str()).x;
                segmentWidths.push_back(width);
                totalWidth += width;
            }
            totalWidth += spacing * (segments.size() - 1);

            float currentX = wpos.x - totalWidth / 2.0f;
            for (size_t i = 0; i < segments.size(); ++i) {
                outlineText(dl, currentX, wpos.y, segments[i].first.c_str(), segments[i].second, 0xFF000000, 1.0f);
                currentX += segmentWidths[i] + spacing;
            }

            wpos.y -= textHeight + 1;
            
            int health = static_cast<int>(std::floor(sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetPlayer(id)->m_fReportedHealth));
            int armour = static_cast<int>(std::floor(sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetPlayer(id)->m_fReportedArmour));

            std::string healthText = std::to_string(health);
            std::string armourText = armour > 0 ? std::to_string(armour) : "";

            float healthWidth = ImGui::CalcTextSize(healthText.c_str()).x;
            float healthX = wpos.x - (healthWidth + ImGui::CalcTextSize(armourText.c_str()).x + spacing) / 2.0f;

            outlineText(dl, healthX, wpos.y, healthText.c_str(), convert(interpolateHealth(static_cast<float>(health))), 0xFF000000, 1.0f);
            outlineText(dl, healthX + healthWidth + spacing, wpos.y, armourText.c_str(), convert(0xFF888888), 0xFF000000, 1.0f);
        }
    }
    else {
        sampapi::CVector headPos;
        sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(id)->m_pPed->GetBonePosition(8, &headPos);
        CVector camPosition = TheCamera.GetPosition();

        float dist = utils->getDistanceBetweenCoords3d(headPos.x, headPos.y, headPos.z, camPosition.x, camPosition.y, camPosition.z);

        if (dist <= gConfig->data["visual"]["distance"].get<float>()) {
            ImVec2 wpos;
            CVector convertedWPos;
            CVector headPosIncludeDistance = CVector(headPos.x, headPos.y, headPos.z + 0.2f + (dist * 0.025f));
            utils->CalcScreenCoords(&headPosIncludeDistance, &convertedWPos);
            wpos = ImVec2(convertedWPos.x, convertedWPos.y);

            float textHeight = ImGui::GetTextLineHeight();
            int spacing = 2;

            std::vector<std::pair<std::string, ImU32>> segments = {
                {sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetName(id), convert(sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(id)->GetColorAsARGB())},
                {"(" + std::to_string(id) + ")", 0xFFFFFFFF},
                {sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(id)->m_nStatus == 2 ? "A" : "", convert(0xFF00FF00)},
                classifyDistance(dist)
            };

            float totalWidth = 0.0f;
            std::vector<float> segmentWidths;
            for (const auto& segment : segments) {
                float width = ImGui::CalcTextSize(segment.first.c_str()).x;
                segmentWidths.push_back(width);
                totalWidth += width;
            }
            totalWidth += spacing * (segments.size() - 1);

            float currentX = wpos.x - totalWidth / 2.0f;
            for (size_t i = 0; i < segments.size(); ++i) {
                outlineText(dl, currentX, wpos.y, segments[i].first.c_str(), segments[i].second, 0xFF000000, 1.0f);
                currentX += segmentWidths[i] + spacing;
            }

            wpos.y -= textHeight + 1;

            int health = static_cast<int>(std::floor(sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(id)->m_fReportedHealth));
            int armour = static_cast<int>(std::floor(sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(id)->m_fReportedArmour));

            std::string healthText = std::to_string(health);
            std::string armourText = armour > 0 ? std::to_string(armour) : "";

            float healthWidth = ImGui::CalcTextSize(healthText.c_str()).x;
            float healthX = wpos.x - (healthWidth + ImGui::CalcTextSize(armourText.c_str()).x + spacing) / 2.0f;

            outlineText(dl, healthX, wpos.y, healthText.c_str(), convert(interpolateHealth(static_cast<float>(health))), 0xFF000000, 1.0f);
            outlineText(dl, healthX + healthWidth + spacing, wpos.y, armourText.c_str(), convert(0xFF888888), 0xFF000000, 1.0f);
        }
    }
    ImGui::PopFont();
}

std::optional<HRESULT> d3dhook::on_lost(const decltype(reset_hook) &hook, IDirect3DDevice9 *device_ptr, D3DPRESENT_PARAMETERS *parameters) {
    ImGui_ImplDX9_InvalidateDeviceObjects();
    return std::nullopt;
}

void d3dhook::on_reset(const decltype(reset_hook) &hook, HRESULT &return_value, IDirect3DDevice9 *device_ptr,
                       D3DPRESENT_PARAMETERS *parameters) {
}

void *get_function_address(const int VTableIndex) {
    return (*reinterpret_cast<void ***>(keyhook::find_device(0x128000)))[VTableIndex];
}

void d3dhook::showCursor(bool state) {
    using RwD3D9GetCurrentD3DDevice_t = LPDIRECT3DDEVICE9(__cdecl *)();

    const auto rwCurrentD3dDevice{reinterpret_cast<
                                          RwD3D9GetCurrentD3DDevice_t>(0x7F9D50U)()};

    if (nullptr == rwCurrentD3dDevice) {
        return;
    }

    static DWORD
            updateMouseProtection,
            rsMouseSetPosProtFirst,
            rsMouseSetPosProtSecond;

    if (state)
    {
        VirtualProtect(reinterpret_cast<void *>(0x53F3C6U), 5U,
                       PAGE_EXECUTE_READWRITE, &updateMouseProtection);

        VirtualProtect(reinterpret_cast<void *>(0x53E9F1U), 5U,
                       PAGE_EXECUTE_READWRITE, &rsMouseSetPosProtFirst);

        VirtualProtect(reinterpret_cast<void *>(0x748A1BU), 5U,
                       PAGE_EXECUTE_READWRITE, &rsMouseSetPosProtSecond);

        // NOP: CPad::UpdateMouse
        *reinterpret_cast<uint8_t *>(0x53F3C6U) = 0xE9U;
        *reinterpret_cast<uint32_t *>(0x53F3C6U + 1U) = 0x15BU;

        // NOP: RsMouseSetPos
        memset(reinterpret_cast<void *>(0x53E9F1U), 0x90, 5U);
        memset(reinterpret_cast<void *>(0x748A1BU), 0x90, 5U);

        rwCurrentD3dDevice->ShowCursor(TRUE);
    }
    else
    {
        // Original: CPad::UpdateMouse
        memcpy(reinterpret_cast<void *>(0x53F3C6U), "\xE8\x95\x6C\x20\x00", 5U);

        // Original: RsMouseSetPos
        memcpy(reinterpret_cast<void *>(0x53E9F1U), "\xE8\xAA\xAA\x0D\x00", 5U);
        memcpy(reinterpret_cast<void *>(0x748A1BU), "\xE8\x80\x0A\xED\xFF", 5U);

        using CPad_ClearMouseHistory_t = void(__cdecl *)();
        using CPad_UpdatePads_t = void(__cdecl *)();

        reinterpret_cast<CPad_ClearMouseHistory_t>(0x541BD0U)();
        reinterpret_cast<CPad_UpdatePads_t>(0x541DD0U)();

        VirtualProtect(reinterpret_cast<void *>(0x53F3C6U), 5U,
                       updateMouseProtection, &updateMouseProtection);

        VirtualProtect(reinterpret_cast<void *>(0x53E9F1U), 5U,
                       rsMouseSetPosProtFirst, &rsMouseSetPosProtFirst);

        VirtualProtect(reinterpret_cast<void *>(0x748A1BU), 5U,
                       rsMouseSetPosProtSecond, &rsMouseSetPosProtSecond);

        rwCurrentD3dDevice->ShowCursor(FALSE);
    }
}

void d3dhook::changeStyleSheet() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    style.Alpha = 1.00f;

    style.WindowRounding = 6.0f;
    style.WindowBorderSize = 0.0f;
    style.WindowPadding = ImVec2(0.0f, 0.0f);
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.ChildRounding = 2.0f;
    style.FrameRounding = 4.0f;
    style.ItemSpacing = ImVec2(0.0f, 0.0f);
    style.ScrollbarSize = 12.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabMinSize = 8.0f;
    style.GrabRounding = 1.0f;
    style.ChildRounding = 6.0f;
    style.FrameBorderSize = 1.0f;

    colors[ImGuiCol_Text] = ImVec4(0.773f, 0.776f, 0.780f, 1.0f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.122f, 0.157f, 0.200f, 1.0f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.122f, 0.157f, 0.200f, 1.0f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.270f, 0.635f, 0.620f, 1.0f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.043f, 0.047f, 0.063f, 0.85f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.270f, 0.635f, 0.620f, 0.45f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.400f, 0.988f, 0.945f, 0.45f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.270f, 0.635f, 0.620f, 0.85f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.400f, 0.988f, 0.945f, 0.6f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.400f, 0.988f, 0.945f, 0.8f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.043f, 0.047f, 0.063f, 0.85f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.270f, 0.635f, 0.620f, 0.85f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.400f, 0.988f, 0.945f, 0.85f);
    colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
}

std::array<uint8_t, 3> convertColorToRGB(sampapi::D3DCOLOR color) {
    std::array<uint8_t, 3> convertedArray = {
        (color >> 24) & 0xFF,
        (color >> 16) & 0xFF,
        (color >> 8) & 0xFF
    };
    return convertedArray;
}

void chams(int id, IDirect3DDevice9* device_ptr) {
    if (device_ptr->TestCooperativeLevel() == D3D_OK)
        if (gSampVersion->getSampVersion() == samp_ver::v037r31) {
            sampapi::v037r3::CPed* pPed = sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(id)->m_pPed;
            if (pPed != nullptr && pPed) {
                CPed* ped = CPools::GetPed(pPed->m_handle);
                if (ped && ped != nullptr && ped->IsAlive()) {
                    DWORD dwCONSTANT, dwARG0, dwARG1, dwARG2, dwCULL;
                    if (gConfig->data["visual"]["chams"]["type"].get<int>() == 0) {
                        device_ptr->GetRenderState(D3DRS_CULLMODE, &dwCULL);
                        device_ptr->GetTextureStageState(0, D3DTSS_CONSTANT, &dwCONSTANT);
                        device_ptr->GetTextureStageState(0, D3DTSS_COLORARG2, &dwARG2);

                        device_ptr->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
                        device_ptr->SetTextureStageState(0, D3DTSS_CONSTANT, 0xFFFFFFFF);
                        device_ptr->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CONSTANT);

                        ped->Render();

                        device_ptr->SetRenderState(D3DRS_CULLMODE, dwCULL);
                        device_ptr->SetTextureStageState(0, D3DTSS_CONSTANT, dwCONSTANT);
                        device_ptr->SetTextureStageState(0, D3DTSS_COLORARG2, dwARG2);
                    }
                    else {
                        device_ptr->GetTextureStageState(0, D3DTSS_CONSTANT, &dwCONSTANT);
                        device_ptr->GetTextureStageState(0, D3DTSS_COLORARG0, &dwARG0);
                        device_ptr->GetTextureStageState(0, D3DTSS_COLORARG1, &dwARG1);
                        device_ptr->GetTextureStageState(0, D3DTSS_COLORARG2, &dwARG2);

                        device_ptr->SetTextureStageState(0, D3DTSS_CONSTANT, sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(id)->GetColorAsARGB());
                        device_ptr->SetTextureStageState(0, D3DTSS_COLORARG0, D3DTA_CONSTANT);
                        device_ptr->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_CONSTANT);
                        device_ptr->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CONSTANT);

                        ped->Render();

                        device_ptr->SetTextureStageState(0, D3DTSS_CONSTANT, dwCONSTANT);
                        device_ptr->SetTextureStageState(0, D3DTSS_COLORARG0, dwARG0);
                        device_ptr->SetTextureStageState(0, D3DTSS_COLORARG1, dwARG1);
                        device_ptr->SetTextureStageState(0, D3DTSS_COLORARG2, dwARG2);
                    }
                }
            }
        }
        else {
            sampapi::v037r1::CPed* pPed = sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetPlayer(id)->m_pPed;
            if (pPed != nullptr && pPed) {
                CPed* ped = CPools::GetPed(pPed->m_handle);
                if (ped && ped != nullptr && ped->IsAlive()) {
                    DWORD dwCONSTANT, dwARG0, dwARG1, dwARG2, dwCULL;
                    if (gConfig->data["visual"]["chams"]["type"].get<int>() == 0) {
                        device_ptr->GetRenderState(D3DRS_CULLMODE, &dwCULL);
                        device_ptr->GetTextureStageState(0, D3DTSS_CONSTANT, &dwCONSTANT);
                        device_ptr->GetTextureStageState(0, D3DTSS_COLORARG2, &dwARG2);

                        device_ptr->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
                        device_ptr->SetTextureStageState(0, D3DTSS_CONSTANT, 0xFFFFFFFF);
                        device_ptr->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CONSTANT);

                        ped->Render();

                        device_ptr->SetRenderState(D3DRS_CULLMODE, dwCULL);
                        device_ptr->SetTextureStageState(0, D3DTSS_CONSTANT, dwCONSTANT);
                        device_ptr->SetTextureStageState(0, D3DTSS_COLORARG2, dwARG2);
                    }
                    else {
                        device_ptr->GetTextureStageState(0, D3DTSS_CONSTANT, &dwCONSTANT);
                        device_ptr->GetTextureStageState(0, D3DTSS_COLORARG0, &dwARG0);
                        device_ptr->GetTextureStageState(0, D3DTSS_COLORARG1, &dwARG1);
                        device_ptr->GetTextureStageState(0, D3DTSS_COLORARG2, &dwARG2);

                        device_ptr->SetTextureStageState(0, D3DTSS_CONSTANT, sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetPlayer(id)->GetColorAsARGB());
                        device_ptr->SetTextureStageState(0, D3DTSS_COLORARG0, D3DTA_CONSTANT);
                        device_ptr->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_CONSTANT);
                        device_ptr->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CONSTANT);

                        ped->Render();

                        device_ptr->SetTextureStageState(0, D3DTSS_CONSTANT, dwCONSTANT);
                        device_ptr->SetTextureStageState(0, D3DTSS_COLORARG0, dwARG0);
                        device_ptr->SetTextureStageState(0, D3DTSS_COLORARG1, dwARG1);
                        device_ptr->SetTextureStageState(0, D3DTSS_COLORARG2, dwARG2);
                    }
                }
            }
        }
}

std::vector<std::pair<int, int>> bonePairs = {
            {BONE_NECK, BONE_UPPERTORSO}, {BONE_UPPERTORSO, BONE_RIGHTSHOULDER},
            {BONE_UPPERTORSO, BONE_LEFTSHOULDER}, {BONE_LEFTSHOULDER, BONE_LEFTELBOW},
            {BONE_RIGHTSHOULDER, BONE_RIGHTELBOW}, {BONE_LEFTELBOW, BONE_LEFTWRIST},
            {BONE_RIGHTELBOW, BONE_RIGHTWRIST}, {BONE_LEFTWRIST, BONE_LEFTHAND},
            {BONE_LEFTHAND, BONE_LEFTTHUMB}, {BONE_RIGHTWRIST, BONE_RIGHTHAND},
            {BONE_RIGHTHAND, BONE_RIGHTTHUMB}, {BONE_UPPERTORSO, BONE_SPINE1},
            {BONE_SPINE1, BONE_PELVIS}, {BONE_PELVIS, BONE_RIGHTKNEE},
            {BONE_PELVIS, BONE_LEFTKNEE}, {BONE_RIGHTKNEE, BONE_RIGHTANKLE},
            {BONE_LEFTKNEE, BONE_LEFTANKLE}, {BONE_RIGHTANKLE, BONE_RIGHTFOOT},
            {BONE_LEFTANKLE, BONE_LEFTFOOT}
};

void skeleton(int id) {
    sampapi::CVector playerPosition;
    if (gSampVersion->getSampVersion() == samp_ver::v037r31)
        playerPosition = sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(id)->m_onfootData.m_position;
    else
        playerPosition = sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetPlayer(id)->m_onfootData.m_position;

    sampapi::CVector convertedPlayerPosition;
    utils->CalcScreenCoords(&playerPosition, &convertedPlayerPosition);

    CVector myPos = FindPlayerPed()->m_matrix->pos;
    float dist = utils->getDistanceBetweenCoords3d(playerPosition.x, playerPosition.y, playerPosition.z, myPos.x, myPos.y, myPos.z);

    if (convertedPlayerPosition.z > 1.f && dist <= gConfig->data["visual"]["distance"].get<float>()) {
        ImDrawList* dl = ImGui::GetBackgroundDrawList();

        sampapi::D3DCOLOR playerColor;
        if (gSampVersion->getSampVersion() == samp_ver::v037r31)
            playerColor = sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(id)->GetColorAsRGBA();
        else
            playerColor = sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetPlayer(id)->GetColorAsRGBA();

        std::array<uint8_t, 3> convertedColor = convertColorToRGB(playerColor);
        ImU32 color = IM_COL32(convertedColor[0], convertedColor[1], convertedColor[2], 255);

        sampapi::CVector vecBone[55], convertedVecBone[55];
        for (int iBone = BONE_PELVIS1; iBone <= BONE_RIGHTFOOT; iBone++)
            switch (iBone)
            {
            case 5: case 4: case 22: case 32: case 23: case 33: case 24: case 34: case 25: case 26: case 35: case 36: case 3: case 2: case 52: case 42: case 53: case 43: case 54: case 44:
                if (gSampVersion->getSampVersion() == samp_ver::v037r31)
                    sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(id)->m_pPed->GetBonePosition(iBone, &vecBone[iBone]);
                else
                    sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetPlayer(id)->m_pPed->GetBonePosition(iBone, &vecBone[iBone]);
                utils->CalcScreenCoords(&vecBone[iBone], &convertedVecBone[iBone]);
                break;
            }

        for (const auto& bonePair : bonePairs)
            dl->AddLine(ImVec2(convertedVecBone[bonePair.first].x, convertedVecBone[bonePair.first].y),
                ImVec2(convertedVecBone[bonePair.second].x, convertedVecBone[bonePair.second].y),
                color, gConfig->data["visual"]["skeleton"]["width"].get<float>());
    }
}

void boxes(int id) {
    sampapi::CVector playerPosition;
    if (gSampVersion->getSampVersion() == samp_ver::v037r31)
        playerPosition = sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(id)->m_onfootData.m_position;
    else
        playerPosition = sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetPlayer(id)->m_onfootData.m_position;

    CVector myPos = FindPlayerPed()->m_matrix->pos;
    float dist = utils->getDistanceBetweenCoords3d(playerPosition.x, playerPosition.y, playerPosition.z, myPos.x, myPos.y, myPos.z);

    if (dist <= gConfig->data["visual"]["distance"].get<float>()) {
        ImDrawList* dl = ImGui::GetBackgroundDrawList();

        std::vector<CVector> offsets = {
            {0.0f, -0.3f, -1.0f}, {0.0f, 0.3f, -1.0f}, {0.0f, 0.3f, 1.0f}, {0.0f, -0.3f, 1.0f}
        };

        if (gConfig->data["visual"]["boxes"]["type"].get<int>() == 0) {
            offsets = {
                {0.3f, -0.3f, -1.0f}, {0.3f, 0.3f, -1.0f}, {0.3f, 0.3f, 1.0f}, {0.3f, -0.3f, 1.0f},
                {-0.3f, -0.3f, -1.0f}, {-0.3f, 0.3f, -1.0f}, {-0.3f, 0.3f, 1.0f}, {-0.3f, -0.3f, 1.0f}
            };
        }

        std::vector<std::pair<int, int>> lines = {
            {0, 1}, {1, 2}, {2, 3}, {3, 0}
        };

        if (gConfig->data["visual"]["boxes"]["type"].get<int>() == 0) {
            lines = {
                {0, 1}, {1, 2}, {2, 3}, {3, 0},
                {4, 5}, {5, 6}, {6, 7}, {7, 4},
                {0, 4}, {1, 5}, {2, 6}, {3, 7}
            };
        }

        std::vector<ImVec2> screenCoords(offsets.size());
        for (size_t i = 0; i < offsets.size(); i++) {
            CVector offsetPos = {
                playerPosition.x + offsets[i].x,
                playerPosition.y + offsets[i].y,
                playerPosition.z + offsets[i].z
            };
            CVector convertedPos;
            utils->CalcScreenCoords(&offsetPos, &convertedPos);

            if (convertedPos.z > 1.f) screenCoords[i] = ImVec2(convertedPos.x, convertedPos.y);
        }

        sampapi::D3DCOLOR playerColor;
        if (gSampVersion->getSampVersion() == samp_ver::v037r31)
            playerColor = sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(id)->GetColorAsRGBA();
        else
            playerColor = sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetPlayer(id)->GetColorAsRGBA();
        std::array<uint8_t, 3> convertedColor = convertColorToRGB(playerColor);
        ImU32 color = IM_COL32(convertedColor[0], convertedColor[1], convertedColor[2], 255);

        for (const auto& line : lines)
            dl->AddLine(screenCoords[line.first], screenCoords[line.second], color, 1.0f);
    }
}

void lines(int id) {
    sampapi::CVector playerPosition, myPosition;
    if (gSampVersion->getSampVersion() == samp_ver::v037r31) {
        playerPosition = sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(id)->m_onfootData.m_position;
        myPosition = sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetLocalPlayer()->m_onfootData.m_position;
    }
    else {
        playerPosition = sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetPlayer(id)->m_onfootData.m_position;
        myPosition = sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetLocalPlayer()->m_onfootData.m_position;
    }

    sampapi::CVector convertedPlayerPosition, convertedMyPosition;
    utils->CalcScreenCoords(&playerPosition, &convertedPlayerPosition);
    utils->CalcScreenCoords(&myPosition, &convertedMyPosition);

    float dist = utils->getDistanceBetweenCoords3d(playerPosition.x, playerPosition.y, playerPosition.z, myPosition.x, myPosition.y, myPosition.z);

    if (convertedPlayerPosition.z > 1.f && convertedMyPosition.z > 1.f && dist <= gConfig->data["visual"]["distance"].get<float>()) {
        ImDrawList* dl = ImGui::GetBackgroundDrawList();

        sampapi::D3DCOLOR playerColor;
        if (gSampVersion->getSampVersion() == samp_ver::v037r31)
            playerColor = sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(id)->GetColorAsRGBA();
        else
            playerColor = sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetPlayer(id)->GetColorAsRGBA();
        std::array<uint8_t, 3> convertedColor = convertColorToRGB(playerColor);
        ImU32 color = IM_COL32(convertedColor[0], convertedColor[1], convertedColor[2], 255);

        dl->AddLine(ImVec2(convertedMyPosition.x, convertedMyPosition.y), ImVec2(convertedPlayerPosition.x, convertedPlayerPosition.y), color, gConfig->data["visual"]["lines"]["width"].get<float>());

        if (gConfig->data["visual"]["lines"]["point"].get<bool>())
            dl->AddCircleFilled(ImVec2(convertedPlayerPosition.x, convertedPlayerPosition.y), gConfig->data["visual"]["lines"]["width"].get<float>() + 0.5f, color, 30);
    }
}