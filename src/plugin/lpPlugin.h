#ifndef LPPLUGIN_FILE_H
#define LPPLUGIN_FILE_H

#include <kthook/kthook.hpp>

using CTimerProto = void(__cdecl*)();

class lpPlugin {
public:
    lpPlugin(HMODULE hModule);
    HMODULE hModule;
private:
    kthook::kthook_simple<CTimerProto> hookCTimerUpdate{ reinterpret_cast<void*>(0x561B10) };
    void mainloop(const decltype(hookCTimerUpdate)& hook);
};

#endif