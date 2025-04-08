#ifndef D3D9HOOK_FILE_H
#define D3D9HOOK_FILE_H

#include <kthook/kthook.hpp>
#include <d3d9.h>
#include <windows.h>
#include <imgui.h>

#include "imgui_impl_win32.h"
#include "imgui_impl_dx9.h"

#include "main.h"
#include "keyhook.h"
#include "imgui/imguisdraw.h"
#include "imgui/FaAwesome/IconsFontAwesome6.h"

void *get_function_address(int VTableIndex);

using PresentSignature = HRESULT(__stdcall*)(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);
using ResetSignature = HRESULT(__stdcall*)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
using InitGameInstance = HWND(__cdecl*)(HINSTANCE);
inline kthook::kthook_signal<PresentSignature> present_hook{ get_function_address(17) };
inline kthook::kthook_signal<ResetSignature> reset_hook{ get_function_address(16) };
inline kthook::kthook_signal<InitGameInstance> game_instance_init_hook{ 0x745560 };

namespace d3dhook {
    std::optional<HRESULT> on_present(const decltype(present_hook) &hook, IDirect3DDevice9 *device_ptr, const RECT *, const RECT *, HWND, const RGNDATA *);
    std::optional<HRESULT> on_lost(const decltype(reset_hook) &hook, IDirect3DDevice9 *device_ptr, D3DPRESENT_PARAMETERS *parameters);
    void on_reset(const decltype(reset_hook)& hook, HRESULT& return_value, IDirect3DDevice9* device_ptr, D3DPRESENT_PARAMETERS* parameters);
    void showCursor(bool state);
    void changeStyleSheet();
};

#endif