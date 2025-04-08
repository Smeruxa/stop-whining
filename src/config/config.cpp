#include "config.h"

#include <fstream>
#include <filesystem>

config* gConfig;

config::config(const char* filename) : filename(filename) {
    if (std::filesystem::exists(filename)) {
        std::ifstream file(filename);
        file >> data;
        file.close();
    }
    else {

        // CROSSHAIR

        data["crosshair"] = {};

        data["crosshair"]["FOV"] = {};
        data["crosshair"]["FOV"]["show"] = false;
        data["crosshair"]["FOV"]["radius"] = 400.f;

        data["crosshair"]["silentAim"] = {};
        data["crosshair"]["silentAim"]["state"] = false;
        data["crosshair"]["silentAim"]["skipSameClist"] = false;
        data["crosshair"]["silentAim"]["shootThroughWalls"] = false;
        data["crosshair"]["silentAim"]["maxDistance"] = 100.f;
        data["crosshair"]["silentAim"]["hitChance"] = 100.f;

        data["crosshair"]["smoothAim"] = {};
        data["crosshair"]["smoothAim"]["state"] = false;
        data["crosshair"]["smoothAim"]["skipDead"] = false;
        data["crosshair"]["smoothAim"]["skipSameClist"] = false;
        data["crosshair"]["smoothAim"]["skipAFK"] = false;
        data["crosshair"]["smoothAim"]["smooth"] = 10.f;
        data["crosshair"]["smoothAim"]["maxDistance"] = 100.f;

        data["crosshair"]["triggerBot"] = {};
        data["crosshair"]["triggerBot"]["state"] = false;
        data["crosshair"]["triggerBot"]["skipSameClist"] = false;
        data["crosshair"]["triggerBot"]["activateByKey"] = false;
        data["crosshair"]["triggerBot"]["maxDistance"] = 100.f;
        data["crosshair"]["triggerBot"]["delay"] = 0.f;
        data["crosshair"]["triggerBot"]["button"] = ImGuiKey_P;

        data["crosshair"]["noSpread"] = {};
        data["crosshair"]["noSpread"]["state"] = false;
        data["crosshair"]["noSpread"]["spread"] = 50.f;

        data["crosshair"]["noCamRestore"] = {};
        data["crosshair"]["noCamRestore"]["state"] = false;

        // PLAYER

        data["player"] = {};

        data["player"]["airbrake"] = {};
        data["player"]["airbrake"]["state"] = false;
        data["player"]["airbrake"]["speed"] = 2.f;
        data["player"]["airbrake"]["button"] = ImGuiKey_RightShift;

        data["player"]["antistun"] = {};
        data["player"]["antistun"]["state"] = false;
        data["player"]["antistun"]["activateByKey"] = false;
        data["player"]["antistun"]["button"] = ImGuiKey_1;
        data["player"]["antistun"]["chance"] = 100.f;

        data["player"]["sbiv"] = {};
        data["player"]["sbiv"]["state"] = false;
        data["player"]["sbiv"]["button"] = ImGuiKey_2;

        data["player"]["fastRotation"] = {};
        data["player"]["fastRotation"]["state"] = false;
        data["player"]["fastRotation"]["speed"] = 20.f;

        data["player"]["megaJump"] = {};
        data["player"]["megaJump"]["state"] = false;
        data["player"]["megaJump"]["increase"] = 2.0f;

        data["player"]["infinityRun"] = false;
        data["player"]["infinityOxygen"] = false;
        data["player"]["noFall"] = false;
        data["player"]["fastRespawn"] = false;
        data["player"]["antiBunnyHop"] = false;
        data["player"]["antiFreeze"] = false;

        // VEHICLE

        data["vehicle"] = {};

        data["vehicle"]["flip"] = {};
        data["vehicle"]["flip"]["state"] = false;
        data["vehicle"]["flip"]["button"] = ImGuiKey_B;

        data["vehicle"]["noBike"] = {};
        data["vehicle"]["noBike"]["state"] = false;
        data["vehicle"]["noBike"]["turnOffInWater"] = true;

        data["vehicle"]["godmode"] = {};
        data["vehicle"]["godmode"]["state"] = false;
        data["vehicle"]["godmode"]["constHp"] = true;

        data["vehicle"]["fastExit"] = {};
        data["vehicle"]["fastExit"]["state"] = false;
        data["vehicle"]["fastExit"]["button"] = ImGuiKey_Z;

        data["vehicle"]["drift"] = {};
        data["vehicle"]["drift"]["state"] = false;
        data["vehicle"]["drift"]["button"] = ImGuiKey_X;

        data["vehicle"]["jumpCar"] = {};
        data["vehicle"]["jumpCar"]["state"] = false;
        data["vehicle"]["jumpCar"]["button"] = ImGuiKey_E;
        data["vehicle"]["jumpCar"]["zPosition"] = 1.5f; // 0.2f - min, 2.f - max 

        data["vehicle"]["underwaterDrive"] = {};
        data["vehicle"]["underwaterDrive"]["state"] = false;

        data["vehicle"]["bigJumpBMX"] = {};
        data["vehicle"]["bigJumpBMX"]["state"] = false;

        data["vehicle"]["overwaterDrive"] = {};
        data["vehicle"]["overwaterDrive"]["state"] = false;

        data["vehicle"]["speedhack"] = {};
        data["vehicle"]["speedhack"]["state"] = false;
        data["vehicle"]["speedhack"]["maxSpeed"] = 180.f;
        data["vehicle"]["speedhack"]["accuracy"] = 0.8f; // 0.1f - minimum, 1.0 - maximum

        // ADDITIONAL

        data["additional"] = {};

        data["additional"]["crosshair"] = {};
        data["additional"]["crosshair"]["state"] = false;
        data["additional"]["crosshair"]["style"] = 0;

        data["additional"]["server"] = {};
        data["additional"]["server"]["time"] = 0;
        data["additional"]["server"]["weather"] = 0;

        data["additional"]["FOV"] = {};
        data["additional"]["FOV"]["state"] = false;
        data["additional"]["FOV"]["num"] = 70.f;

        // VISUAL

        data["visual"] = {};
        data["visual"]["distance"] = 300.f;

        data["visual"]["boxes"] = {};
        data["visual"]["boxes"]["state"] = false;
        data["visual"]["boxes"]["type"] = 0;

        data["visual"]["lines"] = {};
        data["visual"]["lines"]["state"] = false;
        data["visual"]["lines"]["width"] = 1.f;
        data["visual"]["lines"]["point"] = false;

        data["visual"]["skeleton"] = {};
        data["visual"]["skeleton"]["state"] = false;
        data["visual"]["skeleton"]["width"] = 1.f;

        data["visual"]["chams"] = {};
        data["visual"]["chams"]["state"] = false;
        data["visual"]["chams"]["type"] = 0;

        data["visual"]["nametags"] = {};
        data["visual"]["nametags"]["state"] = false;
        data["visual"]["nametags"]["defaultNametags"] = true;

        save();
    }
}

bool config::save() {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    file << std::setw(4) << data;

    file.close();
    return true;
}
