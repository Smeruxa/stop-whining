#include "keyhook.h"
#include "main.h"
#include "plugin/cheats/utils/lpUtils.h"

#include <imgui.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

HRESULT __stdcall keyhook::on_wndproc(const decltype(wndproc_hook) &hook, HWND hwnd, const UINT uMsg,
                                      const WPARAM wParam, const LPARAM lParam) {
    ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);

    if (uMsg == WM_KEYDOWN)
        utils->downKeys.insert(static_cast<int>(wParam));
    else if (uMsg == WM_KEYUP) {
        if (wParam == VK_F11)
            variables.is_open_menu ^= true;

        utils->downKeys.erase(static_cast<int>(wParam));
    }

    if (const auto &io = ImGui::GetIO(); io.WantCaptureKeyboard || io.WantCaptureMouse)
        return 1;
    return hook.get_trampoline()(hwnd, uMsg, wParam, lParam);
}

std::uintptr_t keyhook::find_device(const std::uint32_t LenId) {
    static std::uintptr_t base = [](const std::size_t Len) {
        std::string path_to(MAX_PATH, '\0');
        if (const auto size = GetSystemDirectoryA(path_to.data(), MAX_PATH)) {
            path_to.resize(size);
            path_to += "\\d3d9.dll";
            auto dwObjBase = reinterpret_cast<std::uintptr_t>(LoadLibraryA(path_to.c_str()));
            while (dwObjBase++ < dwObjBase + Len) {
                if (*reinterpret_cast<std::uint16_t *>(dwObjBase + 0x00) == 0x06C7 &&
                    *reinterpret_cast<std::uint16_t *>(dwObjBase + 0x06) == 0x8689 &&
                    *reinterpret_cast<std::uint16_t *>(dwObjBase + 0x0C) == 0x8689) {
                    dwObjBase += 2;
                    break;
                }
            }
            return dwObjBase;
        }
        return static_cast<std::uintptr_t>(0);
    }(LenId);
    return base;
}