#include "lpPlugin.h"
#include "sampver/sampver.h"

#include "d3d9/d3dhook.h"
#include "rakhook/rakhookinstall.h"
#include "imgui/imguisdraw.h"
#include "plugin/cheats/player/lpPlayer.h"
#include "plugin/cheats/vehicle/lpVehicle.h"
#include "plugin/cheats/additional/lpAdditional.h"
#include "plugin/cheats/crosshair/lpCrosshair.h"
#include "plugin/cheats/utils/lpUtils.h"

#include <sampapi/CChat.h>
#include <sampapi/CInput.h>
#include <sampapi/CNetGame.h>
#include <RakHook/rakhook.hpp>
#include <RakNet/PacketEnumerations.h>
#include <common.h>

#include <CWeather.h>
#include <CWorld.h>
#include <CCamera.h>

/*
fixed: speedhack, airbrake in vehicle
*/

lpPlugin::lpPlugin(HMODULE hndl) : hModule(hndl) {
    using namespace std::placeholders;
    hookCTimerUpdate.set_cb(std::bind(&lpPlugin::mainloop, this, _1));
    hookCTimerUpdate.install();
}

void lpPlugin::mainloop(const decltype(hookCTimerUpdate)& hook) {
    static bool inited = false;
    if (!inited && sampapi::v037r3::RefChat() != nullptr && rakhook::initialize()) {
        using namespace std::placeholders;

        variables.s_samp = (DWORD)LoadLibraryA("samp.dll");

        gSampVersion = new sampver();
        samp_ver sampVersion = gSampVersion->getSampVersion();

        utils = new lpUtils();
        r3 = new lpUtils::r3();
        r1 = new lpUtils::r1();

        gImguiDraw = new imguiDraw();
        gRakhook = new rakhookInstall();

        gPlayer = new lpPlayer();
        gVehicle = new lpVehicle();
        gCrosshair = new lpCrosshair();
        gAdditional = new lpAdditional();

        present_hook.before += d3dhook::on_present;
        reset_hook.before += d3dhook::on_lost;
        reset_hook.after += d3dhook::on_reset;

        inited = true;
    }
    else {
        variables.tasklist.process();

        gPlayer->callFunctions();
        gVehicle->callFunctions();
        gCrosshair->callFunctions();
        gAdditional->callFunctions();
    }
    return hook.get_trampoline()();
}