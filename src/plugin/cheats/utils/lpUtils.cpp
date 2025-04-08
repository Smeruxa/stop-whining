#include "lpUtils.h"

#include <unordered_map>
#include <CCamera.h>
#include <d3dx9.h>
#include <ePedBones.h>

lpUtils* utils;
lpUtils::r3* r3;
lpUtils::r1* r1;

void lpUtils::SendKeyInput(WPARAM keyCode) {
    INPUT input[2] = {};

    // Нажатие клавиши
    input[0].type = INPUT_KEYBOARD;
    input[0].ki.wVk = keyCode;
    input[0].ki.dwFlags = 0;

    // Отпускание клавиши
    input[1].type = INPUT_KEYBOARD;
    input[1].ki.wVk = keyCode;
    input[1].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(2, input, sizeof(INPUT));
}

void lpUtils::CalcScreenCoords(sampapi::CVector* vecWorld, sampapi::CVector* vecScreen) {
    D3DXMATRIX m((float*)(0xB6FA2C));

    DWORD* dwLenX = (DWORD*)(0xC17044);
    DWORD* dwLenY = (DWORD*)(0xC17048);

    vecScreen->x = (vecWorld->z * m._31) + (vecWorld->y * m._21) + (vecWorld->x * m._11) + m._41;
    vecScreen->y = (vecWorld->z * m._32) + (vecWorld->y * m._22) + (vecWorld->x * m._12) + m._42;
    vecScreen->z = (vecWorld->z * m._33) + (vecWorld->y * m._23) + (vecWorld->x * m._13) + m._43;

    double fRecip = (double)1.0 / vecScreen->z;
    vecScreen->x *= (float)(fRecip * (*dwLenX));
    vecScreen->y *= (float)(fRecip * (*dwLenY));
}

void lpUtils::CalcScreenCoords(CVector* vecWorld, CVector* vecScreen)
{
    D3DXMATRIX m((float*)(0xB6FA2C));

    DWORD* dwLenX = (DWORD*)(0xC17044);
    DWORD* dwLenY = (DWORD*)(0xC17048);

    vecScreen->x = (vecWorld->z * m._31) + (vecWorld->y * m._21) + (vecWorld->x * m._11) + m._41;
    vecScreen->y = (vecWorld->z * m._32) + (vecWorld->y * m._22) + (vecWorld->x * m._12) + m._42;
    vecScreen->z = (vecWorld->z * m._33) + (vecWorld->y * m._23) + (vecWorld->x * m._13) + m._43;

    double fRecip = (double)1.0 / vecScreen->z;
    vecScreen->x *= (float)(fRecip * (*dwLenX));
    vecScreen->y *= (float)(fRecip * (*dwLenY));
}

void lpUtils::getCrosshairPosition(CVector* vec_out) {
    CVector tmp_vec;
    TheCamera.Find3rdPersonCamTargetVector(15.0f, tmp_vec, &tmp_vec, vec_out);
}

void lpUtils::setCrosshairState(bool param) {
    static std::vector<DWORD> crosshair = { 5825260, 5825281, 5825270, 5825265, 5825471, 5825498, 5825489, 5825480, 5825568, 5825587, 5825578, 5825573, 5825633, 5825660, 5825651, 5825638, 5825638, 5825159, 5825180, 5825175, 5825170 };
    BYTE value = param ? 255 : 0;
    for (DWORD val : crosshair) {
        *(DWORD*)val = value;
    }
}

int lpUtils::getVirtualKey(ImGuiKey key) {
    static std::unordered_map<ImGuiKey, int> kImGui = {
        {ImGuiKey_Tab, 9},
        {ImGuiKey_LeftArrow, 37},
        {ImGuiKey_RightArrow, 39},
        {ImGuiKey_UpArrow, 38},
        {ImGuiKey_DownArrow, 40},
        {ImGuiKey_PageUp, 33},
        {ImGuiKey_PageDown, 34},
        {ImGuiKey_Home, 36},
        {ImGuiKey_End, 35},
        {ImGuiKey_Insert, 45},
        {ImGuiKey_Delete, 46},
        {ImGuiKey_Backspace, 8},
        {ImGuiKey_Space, 32},
        {ImGuiKey_Enter, 13},
        {ImGuiKey_Escape, 27},
        {ImGuiKey_A, 65},
        {ImGuiKey_B, 66},
        {ImGuiKey_C, 67},
        {ImGuiKey_D, 68},
        {ImGuiKey_E, 69},
        {ImGuiKey_F, 70},
        {ImGuiKey_G, 71},
        {ImGuiKey_H, 72},
        {ImGuiKey_I, 73},
        {ImGuiKey_J, 74},
        {ImGuiKey_K, 75},
        {ImGuiKey_L, 76},
        {ImGuiKey_M, 77},
        {ImGuiKey_N, 78},
        {ImGuiKey_O, 79},
        {ImGuiKey_P, 80},
        {ImGuiKey_Q, 81},
        {ImGuiKey_R, 82},
        {ImGuiKey_S, 83},
        {ImGuiKey_T, 84},
        {ImGuiKey_U, 85},
        {ImGuiKey_V, 86},
        {ImGuiKey_W, 87},
        {ImGuiKey_X, 88},
        {ImGuiKey_Y, 89},
        {ImGuiKey_Z, 90},
        {ImGuiKey_0, 48},
        {ImGuiKey_1, 49},
        {ImGuiKey_2, 50},
        {ImGuiKey_3, 51},
        {ImGuiKey_4, 52},
        {ImGuiKey_5, 53},
        {ImGuiKey_6, 54},
        {ImGuiKey_7, 55},
        {ImGuiKey_8, 56},
        {ImGuiKey_9, 57},
        {ImGuiKey_F1, 112},
        {ImGuiKey_F2, 113},
        {ImGuiKey_F3, 114},
        {ImGuiKey_F4, 115},
        {ImGuiKey_F5, 116},
        {ImGuiKey_F6, 117},
        {ImGuiKey_F7, 118},
        {ImGuiKey_F8, 119},
        {ImGuiKey_F9, 120},
        {ImGuiKey_F10, 121},
        {ImGuiKey_F11, 122},
        {ImGuiKey_F12, 123},
        {ImGuiKey_F13, 124},
        {ImGuiKey_F14, 125},
        {ImGuiKey_F15, 126},
        {ImGuiKey_F16, 127},
        {ImGuiKey_F17, 128},
        {ImGuiKey_F18, 129},
        {ImGuiKey_F19, 130},
        {ImGuiKey_F20, 131},
        {ImGuiKey_F21, 132},
        {ImGuiKey_F22, 133},
        {ImGuiKey_F23, 134},
        {ImGuiKey_F24, 135},
        {ImGuiKey_Apostrophe, 192},
        {ImGuiKey_Comma, 188},
        {ImGuiKey_Minus, 189},
        {ImGuiKey_Period, 190},
        {ImGuiKey_Slash, 191},
        {ImGuiKey_Semicolon, 186},
        {ImGuiKey_Equal, 187},
        {ImGuiKey_LeftBracket, 219},
        {ImGuiKey_Backslash, 220},
        {ImGuiKey_RightBracket, 221},
        {ImGuiKey_GraveAccent, 192},
        {ImGuiKey_CapsLock, 20},
        {ImGuiKey_ScrollLock, 145},
        {ImGuiKey_NumLock, 144},
        {ImGuiKey_PrintScreen, 44},
        {ImGuiKey_Pause, 19},
        {ImGuiKey_Keypad0, 96},
        {ImGuiKey_Keypad1, 97},
        {ImGuiKey_Keypad2, 98},
        {ImGuiKey_Keypad3, 99},
        {ImGuiKey_Keypad4, 100},
        {ImGuiKey_Keypad5, 101},
        {ImGuiKey_Keypad6, 102},
        {ImGuiKey_Keypad7, 103},
        {ImGuiKey_Keypad8, 104},
        {ImGuiKey_Keypad9, 105},
        {ImGuiKey_KeypadDecimal, 110},
        {ImGuiKey_KeypadDivide, 111},
        {ImGuiKey_KeypadMultiply, 106},
        {ImGuiKey_KeypadSubtract, 109},
        {ImGuiKey_KeypadAdd, 107},
        {ImGuiKey_KeypadEnter, 13},
        {ImGuiKey_KeypadEqual, 187},
        {ImGuiKey_LeftShift, VK_LSHIFT},
        {ImGuiKey_RightShift, VK_RSHIFT},
        {ImGuiKey_LeftCtrl, VK_LCONTROL},
        {ImGuiKey_RightCtrl, VK_RCONTROL},
        {ImGuiKey_LeftAlt, VK_LMENU},
        {ImGuiKey_RightAlt, VK_RMENU},
        {ImGuiKey_LeftSuper, VK_LWIN},
        {ImGuiKey_RightSuper, VK_RWIN},
        {ImGuiKey_MouseLeft, VK_LBUTTON},
        {ImGuiKey_MouseRight, VK_RBUTTON},
        {ImGuiKey_MouseMiddle, VK_MBUTTON},
        {ImGuiKey_MouseX1, VK_XBUTTON1},
        {ImGuiKey_MouseX2, VK_XBUTTON2},
    };

	auto it = kImGui.find(key);
	if (it != kImGui.end())
		return it->second;
	else
		return 0;
}

float lpUtils::getDistanceBetweenCoords3d(float x, float y, float z, float x1, float y1, float z1) {
    return sqrt(((x1 - x) * (x1 - x)) + ((y1 - y) * (y1 - y)) + ((z1 - z) * (z1 - z)));
}

float lpUtils::getDistanceBetweenCoords2d(float x, float y, float x1, float y1) {
    return sqrt(((x1 - x) * (x1 - x)) + ((y1 - y) * (y1 - y)));
}

bool lpUtils::isKeyPressed(int virtualKeyCode) {
    static std::unordered_map<int, bool> keyPreviouslyPressedMap;

    bool keyCurrentlyPressed = (GetAsyncKeyState(virtualKeyCode) & 0x8000) != 0;
    bool keyPreviouslyPressed = keyPreviouslyPressedMap[virtualKeyCode];

    if (keyCurrentlyPressed && !keyPreviouslyPressed) {
        keyPreviouslyPressedMap[virtualKeyCode] = true;
        return true;
    }

    keyPreviouslyPressedMap[virtualKeyCode] = keyCurrentlyPressed;
    return false;
}

bool lpUtils::isKeyDown(int key) {
	return downKeys.count(key) > 0;
}

std::pair<int, sampapi::v037r3::CPed*> lpUtils::r3::getNearCharToCenter(float radius, float centerX, float centerY, float maxDistance) {
    std::vector<playerDistance> players;

    for (int i = 0; i < sampapi::v037r3::RefNetGame()->GetPlayerPool()->MAX_PLAYERS; i++) {
        if (sampapi::v037r3::RefNetGame()->GetPlayerPool()->IsConnected(i)) {
            sampapi::v037r3::CPed* pPed = sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(i)->m_pPed;
            if (pPed && pPed != nullptr && !pPed->IsDead()) {
                sampapi::CVector position = sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(i)->m_onfootData.m_position;

                if (utils->getDistanceBetweenCoords3d(position.x, position.y, position.z, FindPlayerCoors(-1).x, FindPlayerCoors(-1).y, FindPlayerCoors(-1).z) <= maxDistance) {
                    sampapi::CVector convertedPosition;
                    utils->CalcScreenCoords(&position, &convertedPosition);

                    if (convertedPosition.z > 1.f && sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(i)->m_pPed->IsOnScreen()) {
                        float distance = utils->getDistanceBetweenCoords2d(centerX, centerY, convertedPosition.x, convertedPosition.y);

                        if (distance <= radius)
                            players.push_back({ distance, i, pPed });
                    }
                }
            }
        }
    }

    if (!players.empty()) {
        std::sort(players.begin(), players.end(), [](playerDistance a, playerDistance b) {
            return a.distance < b.distance;
            });
        return std::pair<int, sampapi::v037r3::CPed*>(players.front().playerId, players.front().ped);
    }

    return std::pair<int, sampapi::v037r3::CPed*>(-1, nullptr);
}

int lpUtils::r3::getNearBone(sampapi::v037r3::CPed* ped, float centerX, float centerY) {
    static std::vector<int> iBoneList = { BONE_PELVIS1, BONE_PELVIS, BONE_SPINE1, BONE_UPPERTORSO, BONE_NECK, BONE_HEAD2, BONE_HEAD1, BONE_HEAD, BONE_RIGHTUPPERTORSO, BONE_RIGHTSHOULDER, BONE_RIGHTELBOW, BONE_LEFTUPPERTORSO, BONE_LEFTSHOULDER, BONE_LEFTELBOW, BONE_LEFTHIP, BONE_LEFTKNEE, BONE_RIGHTHIP, BONE_RIGHTKNEE };
    std::vector<boneDistance> bones;

    for (int bone : iBoneList) {
        sampapi::CVector position;
        sampapi::CVector convertedPosition;
        ped->GetBonePosition(bone, &position);
        utils->CalcScreenCoords(&position, &convertedPosition);

        if (convertedPosition.z > 1.f) {
            float distance = utils->getDistanceBetweenCoords2d(centerX, centerY, convertedPosition.x, convertedPosition.y);
            bones.push_back({ distance, bone });
        }
    }

    if (!bones.empty()) {
        std::sort(bones.begin(), bones.end(), [](boneDistance a, boneDistance b) {
            return a.distance < b.distance;
            });
        return bones.front().bone;
    }

    return -1;
}

std::pair<int, sampapi::v037r1::CPed*> lpUtils::r1::getNearCharToCenter(float radius, float centerX, float centerY, float maxDistance) {
    std::vector<playerDistance> players;

    for (int i = 0; i < sampapi::v037r1::RefNetGame()->GetPlayerPool()->MAX_PLAYERS; i++) {
        if (sampapi::v037r1::RefNetGame()->GetPlayerPool()->IsConnected(i)) {
            sampapi::v037r1::CPed* pPed = sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetPlayer(i)->m_pPed;
            if (pPed && pPed != nullptr && !pPed->IsDead()) {
                sampapi::CVector position = sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetPlayer(i)->m_onfootData.m_position;

                if (utils->getDistanceBetweenCoords3d(position.x, position.y, position.z, FindPlayerCoors(-1).x, FindPlayerCoors(-1).y, FindPlayerCoors(-1).z) <= maxDistance) {
                    sampapi::CVector convertedPosition;
                    utils->CalcScreenCoords(&position, &convertedPosition);

                    if (convertedPosition.z > 1.f && sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetPlayer(i)->m_pPed->IsOnScreen()) {
                        float distance = utils->getDistanceBetweenCoords2d(centerX, centerY, convertedPosition.x, convertedPosition.y);

                        if (distance <= radius)
                            players.push_back({ distance, i, pPed });
                    }
                }
            }
        }
    }

    if (!players.empty()) {
        std::sort(players.begin(), players.end(), [](playerDistance a, playerDistance b) {
            return a.distance < b.distance;
            });
        return std::pair<int, sampapi::v037r1::CPed*>(players.front().playerId, players.front().ped);
    }

    return std::pair<int, sampapi::v037r1::CPed*>(-1, nullptr);
}

int lpUtils::r1::getNearBone(sampapi::v037r1::CPed* ped, float centerX, float centerY) {
    static std::vector<int> iBoneList = { BONE_PELVIS1, BONE_PELVIS, BONE_SPINE1, BONE_UPPERTORSO, BONE_NECK, BONE_HEAD2, BONE_HEAD1, BONE_HEAD, BONE_RIGHTUPPERTORSO, BONE_RIGHTSHOULDER, BONE_RIGHTELBOW, BONE_LEFTUPPERTORSO, BONE_LEFTSHOULDER, BONE_LEFTELBOW, BONE_LEFTHIP, BONE_LEFTKNEE, BONE_RIGHTHIP, BONE_RIGHTKNEE };
    std::vector<boneDistance> bones;

    for (int bone : iBoneList) {
        sampapi::CVector position;
        sampapi::CVector convertedPosition;
        ped->GetBonePosition(bone, &position);
        utils->CalcScreenCoords(&position, &convertedPosition);

        if (convertedPosition.z > 1.f) {
            float distance = utils->getDistanceBetweenCoords2d(centerX, centerY, convertedPosition.x, convertedPosition.y);
            bones.push_back({ distance, bone });
        }
    }

    if (!bones.empty()) {
        std::sort(bones.begin(), bones.end(), [](boneDistance a, boneDistance b) {
            return a.distance < b.distance;
            });
        return bones.front().bone;
    }

    return -1;
}