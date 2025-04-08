#include "plugin/lpPlugin.h"
#include "main.h"

#include "imgui/HotKey/ImHotKey.h"
#include "config/config.h"

vars variables;
std::unique_ptr<lpPlugin> plugin;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        plugin = std::make_unique<lpPlugin>(hModule);

        //AllocConsole();
        //freopen("CONIN$", "r", stdin);
        //freopen("CONOUT$", "w", stdout);
        //freopen("CONOUT$", "w", stderr);

        gHotKey = new ImHotKey();
        gConfig = new config("SWCFG.json");
    }
    return TRUE;
}