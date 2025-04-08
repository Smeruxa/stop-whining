#include "lpCrosshair.h"
#include "config/config.h"
#include "main.h"
#include "DirectX/d3dx9.h"
#include "plugin/cheats/utils/lpUtils.h"
#include "sampver/sampver.h"
#include "secure/Secure.h"
#include "secure/memory/Memory.h"

#include <sampapi/CChat.h>
#include <sampapi/CNetGame.h>

#include <CCamera.h>
#include <CWorld.h>
#include <ePedBones.h>
#include <CObject.h>
#include <CPools.h>
#include <detours.h>

#include <Windows.h>
#include <CWeaponInfo.h>
#include <CWeapon.h>

lpCrosshair* gCrosshair;

static lpCrosshair::DoBulletImpact fpDoImpact = (lpCrosshair::DoBulletImpact)(0x73B550);
static lpCrosshair::FireInstantHit fpFireHit = (lpCrosshair::FireInstantHit)(0x73FB10);

sampapi::GTAREF silHandle = -1;
int silId = -1;
int silBone = -1;
bool silChance = false;

bool isTargeting = false;
CVector2D vecCrosshair;
std::vector<int> iBoneList = { BONE_PELVIS1, BONE_PELVIS, BONE_SPINE1, BONE_UPPERTORSO, BONE_NECK, BONE_HEAD2, BONE_HEAD1, BONE_HEAD, BONE_RIGHTUPPERTORSO, BONE_RIGHTSHOULDER, BONE_RIGHTELBOW, BONE_LEFTUPPERTORSO, BONE_LEFTSHOULDER, BONE_LEFTELBOW, BONE_LEFTHIP, BONE_LEFTKNEE, BONE_RIGHTHIP, BONE_RIGHTKNEE };

bool __fastcall lpCrosshair::FireInstantHitHooked(void* _this, void* EDX, CEntity* firingEntity, CVector* origin, CVector* muzzlePosn, CEntity* targetEntity, CVector* target, CVector* originForDriveBy, bool arg6, bool muzzle) {
    if (firingEntity == (CEntity*)FindPlayerPed() && gConfig->data["crosshair"]["silentAim"]["state"].get<bool>() && silHandle != -1 && silId != -1 && silBone != -1 && silChance) {
        CPed* ped = CPools::GetPed(silHandle);
        if (ped && ped != nullptr) {
            RwV3d bone;
            ped->GetBonePosition(bone, silBone, true);

            CVector position = CVector(bone.x, bone.y, bone.z);
            target = &position;

            pSecure->memcpy_safe((void*)0x740B4E, gConfig->data["crosshair"]["silentAim"]["shootThroughWalls"].get<bool>() ? "\x6A\x00\x6A\x00" : "\x6A\x01\x6A\x01", 4);
        }
    }
    else
        Memory::memcpy_safe((void*)0x740B4E, "\x6A\x01\x6A\x01", 4);
    return fpFireHit(_this, EDX, firingEntity, origin, muzzlePosn, targetEntity, target, originForDriveBy, arg6, muzzle);
}

void __fastcall lpCrosshair::DoBulletImpactHooked(void* weapon, void* EDX, CEntity* owner, CEntity* victim, CVector* startPoint, CVector* endPoint, CColPoint* colPoint, int arg5) {
    if (victim != nullptr && owner == *(CEntity**)0xB6F5F0 && gConfig->data["crosshair"]["silentAim"]["state"].get<bool>() && silHandle != -1 && silId != -1 && silBone != -1 && silChance) {
        CPed* ped = CPools::GetPed(silHandle);
        if (ped && ped != nullptr) {
            RwV3d bone;
            ped->GetBonePosition(bone, silBone, true);

            CVector position = CVector(bone.x, bone.y, bone.z);
            colPoint->m_vecPoint = position;

            pSecure->memcpy_safe((void*)0x736212, gConfig->data["crosshair"]["silentAim"]["shootThroughWalls"].get<bool>() ? "\x6A\x00\x6A\x00" : "\x6A\x01\x6A\x01", 4);
        }
    }
    else
        Memory::memcpy_safe((void*)0x736212, "\x6A\x01\x6A\x01", 4);
    return fpDoImpact(weapon, EDX, owner, victim, startPoint, endPoint, colPoint, arg5);
}

lpCrosshair::lpCrosshair() {
    DetourRestoreAfterWith();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    pSecure->SDetourAttach(&(PVOID&)fpDoImpact, DoBulletImpactHooked);
    pSecure->SDetourAttach(&(PVOID&)fpFireHit, FireInstantHitHooked);
    DetourTransactionCommit();
}

void lpCrosshair::callFunctions() {
    isTargeting = (TheCamera.m_aCams[0].m_nMode == 53 || TheCamera.m_aCams[0].m_nMode == 7);

    if (FindPlayerPed()->m_nWeaponModelId != 34) {
        vecCrosshair.x = variables.displaySize[0] * 0.53f;
        vecCrosshair.y = variables.displaySize[1] * 0.4f;
    }
    else {
        vecCrosshair.x = variables.displaySize[0] / 2.0f;
        vecCrosshair.y = variables.displaySize[1] / 2.0f;
    }

    variables.circlePos[0] = vecCrosshair.x;
    variables.circlePos[1] = vecCrosshair.y;

    *(uint8_t*)0x5231A6 = (FindPlayerPed()->IsAlive() && gConfig->data["crosshair"]["noCamRestore"]["state"].get<bool>()) ? 0x90 : 0x75;

    silentAim();
	noSpread();
    smoothAim();
    triggerBot();
}

void lpCrosshair::silentAim() {
    if (gConfig->data["crosshair"]["silentAim"]["state"].get<bool>() && variables.displaySize[0] != variables.displaySize[1] != 0 && FindPlayerPed()->IsAlive())
        if (gSampVersion->getSampVersion() == samp_ver::v037r31) {
            std::pair<int, sampapi::v037r3::CPed*> nearId = r3->getNearCharToCenter(gConfig->data["crosshair"]["FOV"]["radius"].get<float>(), vecCrosshair.x, vecCrosshair.y, gConfig->data["crosshair"]["silentAim"]["maxDistance"].get<float>());
            if (nearId.first != -1) {
                int nearBone = r3->getNearBone(nearId.second, vecCrosshair.x, vecCrosshair.y);
                if (nearBone != -1) {
                    if ((gConfig->data["crosshair"]["silentAim"]["skipSameClist"].get<bool>() && sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(nearId.first)->GetColorAsRGBA() == sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetLocalPlayer()->GetColorAsRGBA()) ? false : true) {
                        silHandle = nearId.second->m_handle;
                        silId = nearId.first;
                        silBone = nearBone;
                        silChance = rand() % 100 <= gConfig->data["crosshair"]["silentAim"]["hitChance"].get<float>();
                    }
                    else {
                        silHandle = silId = silBone = -1;
                        silChance = false;
                    }
                }
                else {
                    silHandle = silId = silBone = -1;
                    silChance = false;
                }
            }
            else {
                silHandle = silId = silBone = -1;
                silChance = false;
            }
        }
        else if (gSampVersion->getSampVersion() == samp_ver::v037r1) {
            std::pair<int, sampapi::v037r1::CPed*> nearId = r1->getNearCharToCenter(gConfig->data["crosshair"]["FOV"]["radius"].get<float>(), vecCrosshair.x, vecCrosshair.y, gConfig->data["crosshair"]["silentAim"]["maxDistance"].get<float>());
            if (nearId.first != -1) {
                int nearBone = r1->getNearBone(nearId.second, vecCrosshair.x, vecCrosshair.y);
                if (nearBone != -1) {
                    if ((gConfig->data["crosshair"]["silentAim"]["skipSameClist"].get<bool>() && sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetPlayer(nearId.first)->GetColorAsRGBA() == sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetLocalPlayer()->GetColorAsRGBA()) ? false : true) {
                        silHandle = nearId.second->m_handle;
                        silId = nearId.first;
                        silBone = nearBone;
                        silChance = rand() % 100 <= gConfig->data["crosshair"]["silentAim"]["hitChance"].get<float>();
                    }
                    else {
                        silHandle = silId = silBone = -1;
                        silChance = false;
                    }
                }
                else {
                    silHandle = silId = silBone = -1;
                    silChance = false;
                }
            }
            else {
                silHandle = silId = silBone = -1;
                silChance = false;
            }
        }
}

void lpCrosshair::triggerBot() {
    if (gConfig->data["crosshair"]["triggerBot"]["state"].get<bool>() && variables.displaySize[0] != variables.displaySize[1] != 0 && FindPlayerPed()->IsAlive() && isTargeting) {
        static ULONGLONG iTick = GetTickCount64();
        float delay = ((gConfig->data["crosshair"]["triggerBot"]["delay"].get<float>() == 0.0f) ? 0.05f : gConfig->data["crosshair"]["triggerBot"]["delay"].get<float>());
        if (GetTickCount64() - iTick >= ((ULONGLONG)delay * 1000.f)) {
            if (gConfig->data["crosshair"]["triggerBot"]["activateByKey"].get<bool>() && !utils->isKeyDown(utils->getVirtualKey(gConfig->data["crosshair"]["triggerBot"]["button"].get<ImGuiKey>())))
                return;
            CVector vecCamera, vecTarget;
            sampapi::CVector vecOrigin;

            if (gSampVersion->getSampVersion() == samp_ver::v037r31) {
                if (sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetLocalPlayer()->m_weaponsData.m_nCurrentWeapon != 34)
                {
                    sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetLocalPlayer()->m_pPed->GetBonePosition((int)BONE_RIGHTWRIST, &vecOrigin);
                    TheCamera.Find3rdPersonCamTargetVector(100.f, CVector(vecOrigin.x, vecOrigin.y, vecOrigin.z), &vecCamera, &vecTarget);
                }
                else
                {
                    CVector vecFront = TheCamera.m_aCams[0].m_vecFront;
                    vecFront.Normalise();
                    vecCamera = *TheCamera.GetGameCamPosition();
                    vecCamera += (vecFront * 2.0f);
                    vecTarget = vecCamera + (vecFront * 100.f);
                }

                CColPoint pCollision; CEntity* pCollisionEntity = NULL;
                bool bCollision = CWorld::ProcessLineOfSight(vecCamera, vecTarget, pCollision, pCollisionEntity, true, true, true, true, true, true, false, true);
                if (bCollision && pCollisionEntity && pCollisionEntity->m_nType == ENTITY_TYPE_PED)
                {
                    for (int i = 0; i < 1004; i++)
                    {
                        if (!sampapi::v037r3::RefNetGame()->GetPlayerPool()->IsConnected(i))
                            continue;

                        sampapi::v037r3::CPed* sPed = sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(i)->m_pPed;
                        if (!sPed)
                            continue;

                        CPed* pPed = CPools::GetPed(sPed->m_handle);
                        if (!pPed)
                            continue;

                        if (pCollisionEntity != pPed || pPed == FindPlayerPed())
                            continue;

                        if (!pPed->IsAlive())
                            continue;

                        if (gConfig->data["crosshair"]["triggerBot"]["skipSameClist"].get<bool>() && sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(i)->GetColorAsRGBA() == sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(i)->GetColorAsRGBA())
                            continue;

                        if (sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(i)->GetDistanceToLocalPlayer() > gConfig->data["crosshair"]["triggerBot"]["maxDistance"].get<float>())
                            continue;

                        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

                        iTick = GetTickCount64();
                        break;
                    }
                }
            }
            else {
                if (sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetLocalPlayer()->m_weaponsData.m_nCurrentWeapon != 34)
                {
                    sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetLocalPlayer()->m_pPed->GetBonePosition((int)BONE_RIGHTWRIST, &vecOrigin);
                    TheCamera.Find3rdPersonCamTargetVector(100.f, CVector(vecOrigin.x, vecOrigin.y, vecOrigin.z), &vecCamera, &vecTarget);
                }
                else
                {
                    CVector vecFront = TheCamera.m_aCams[0].m_vecFront;
                    vecFront.Normalise();
                    vecCamera = *TheCamera.GetGameCamPosition();
                    vecCamera += (vecFront * 2.0f);
                    vecTarget = vecCamera + (vecFront * 100.f);
                }

                CColPoint pCollision; CEntity* pCollisionEntity = NULL;
                bool bCollision = CWorld::ProcessLineOfSight(vecCamera, vecTarget, pCollision, pCollisionEntity, true, true, true, true, true, true, false, true);
                if (bCollision && pCollisionEntity && pCollisionEntity->m_nType == ENTITY_TYPE_PED)
                {
                    for (int i = 0; i < 1004; i++)
                    {
                        if (!sampapi::v037r1::RefNetGame()->GetPlayerPool()->IsConnected(i))
                            continue;

                        sampapi::v037r1::CPed* sPed = sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetPlayer(i)->m_pPed;
                        if (!sPed)
                            continue;

                        CPed* pPed = CPools::GetPed(sPed->m_handle);
                        if (!pPed)
                            continue;

                        if (pCollisionEntity != pPed || pPed == FindPlayerPed())
                            continue;

                        if (!pPed->IsAlive())
                            continue;

                        if (gConfig->data["crosshair"]["triggerBot"]["skipSameClist"].get<bool>() && sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetPlayer(i)->GetColorAsRGBA() == sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetPlayer(i)->GetColorAsRGBA())
                            continue;

                        if (sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetPlayer(i)->GetDistanceToLocalPlayer() > gConfig->data["crosshair"]["triggerBot"]["maxDistance"].get<float>())
                            continue;

                        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

                        iTick = GetTickCount64();
                        break;
                    }
                }
            }
        }
    }
}

void lpCrosshair::smoothAim() {
    if (gConfig->data["crosshair"]["smoothAim"]["state"].get<bool>() && variables.displaySize[0] != variables.displaySize[1] != 0 && FindPlayerPed()->IsAlive()) {
        if (gSampVersion->getSampVersion() == samp_ver::v037r31) {
            std::pair<int, sampapi::v037r3::CPed*> nearId = r3->getNearCharToCenter(gConfig->data["crosshair"]["FOV"]["radius"].get<float>(), vecCrosshair.x, vecCrosshair.y, gConfig->data["crosshair"]["smoothAim"]["maxDistance"].get<float>());
            if (nearId.first != -1) {
                int nearBone = r3->getNearBone(nearId.second, vecCrosshair.x, vecCrosshair.y);
                if (nearBone != -1) {
                    if ((gConfig->data["crosshair"]["smoothAim"]["skipSameClist"].get<bool>() && sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(nearId.first)->GetColorAsRGBA() == sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetLocalPlayer()->GetColorAsRGBA()) ? false : true) {
                        CVector vecCamera, vecTarget;
                        sampapi::CVector vecOrigin;

                        sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetLocalPlayer()->m_pPed->GetBonePosition(BONE_RIGHTWRIST, &vecOrigin);
                        TheCamera.Find3rdPersonCamTargetVector(100.f, CVector(vecOrigin.x, vecOrigin.y, vecOrigin.z), &vecCamera, &vecTarget);

                        sampapi::CVector bonePosition;
                        nearId.second->GetBonePosition(nearBone, &bonePosition);

                        if (CWorld::GetIsLineOfSightClear(vecCamera, CVector(bonePosition.x, bonePosition.y, bonePosition.z), true, true, false, true, true, true, false) && isTargeting) {

                            CVector vecOrigin = *TheCamera.GetGameCamPosition();

                            sampapi::CVector vecTarget;
                            nearId.second->GetBonePosition(nearBone, &vecTarget);

                            CVector vecVector = vecOrigin - CVector(vecTarget.x, vecTarget.y, vecTarget.z);

                            float fFix = 0.0f, fVecX = 0.0f;

                            const BYTE byteWeapon = sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetLocalPlayer()->m_weaponsData.m_nCurrentWeapon;

                            if ((byteWeapon >= 22 && byteWeapon <= 29) || byteWeapon == 32) fFix = 0.04253f;
                            else if (byteWeapon == 30 || byteWeapon == 31) fFix = 0.028f;
                            else if (byteWeapon == 33) fFix = 0.01897f;

                            float fDistX = std::sqrt(vecVector.x * vecVector.x + vecVector.y * vecVector.y);

                            if (vecVector.x <= 0.0 && vecVector.y >= 0.0 || vecVector.x >= 0.0 && vecVector.y >= 0.0)
                                fVecX = (std::acosf(vecVector.x / fDistX) + fFix) - TheCamera.m_aCams[0].m_fHorizontalAngle;
                            if (vecVector.x >= 0.0 && vecVector.y <= 0.0 || vecVector.x <= 0.0 && vecVector.y <= 0.0)
                                fVecX = (-std::acosf(vecVector.x / fDistX) + fFix) - TheCamera.m_aCams[0].m_fHorizontalAngle;

                            if (const float fSmoothX = fVecX / (gConfig->data["crosshair"]["smoothAim"]["smooth"].get<float>() * 2);
                                fSmoothX > -1.0 && fSmoothX < 0.5 && fVecX > -2.0 && fVecX < 2.0)
                                TheCamera.m_aCams[0].m_fHorizontalAngle += fSmoothX;
                        }
                    }
                }
            }
        }
        else if (gSampVersion->getSampVersion() == samp_ver::v037r1) {
            std::pair<int, sampapi::v037r1::CPed*> nearId = r1->getNearCharToCenter(gConfig->data["crosshair"]["FOV"]["radius"].get<float>(), vecCrosshair.x, vecCrosshair.y, gConfig->data["crosshair"]["smoothAim"]["maxDistance"].get<float>());
            if (nearId.first != -1) {
                int nearBone = r1->getNearBone(nearId.second, vecCrosshair.x, vecCrosshair.y);
                if (nearBone != -1) {
                    if ((gConfig->data["crosshair"]["smoothAim"]["skipSameClist"].get<bool>() && sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetPlayer(nearId.first)->GetColorAsRGBA() == sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetLocalPlayer()->GetColorAsRGBA()) ? false : true) {
                        CVector vecCamera, vecTarget;
                        sampapi::CVector vecOrigin;

                        sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetLocalPlayer()->m_pPed->GetBonePosition(BONE_RIGHTWRIST, &vecOrigin);
                        TheCamera.Find3rdPersonCamTargetVector(100.f, CVector(vecOrigin.x, vecOrigin.y, vecOrigin.z), &vecCamera, &vecTarget);

                        sampapi::CVector bonePosition;
                        nearId.second->GetBonePosition(nearBone, &bonePosition);

                        if (CWorld::GetIsLineOfSightClear(vecCamera, CVector(bonePosition.x, bonePosition.y, bonePosition.z), true, true, false, true, true, true, false) && isTargeting) {

                            CVector vecOrigin = *TheCamera.GetGameCamPosition();

                            sampapi::CVector vecTarget;
                            nearId.second->GetBonePosition(nearBone, &vecTarget);

                            CVector vecVector = vecOrigin - CVector(vecTarget.x, vecTarget.y, vecTarget.z);

                            float fFix = 0.0f, fVecX = 0.0f;

                            const BYTE byteWeapon = sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetLocalPlayer()->m_weaponsData.m_nCurrentWeapon;

                            if ((byteWeapon >= 22 && byteWeapon <= 29) || byteWeapon == 32) fFix = 0.04253f;
                            else if (byteWeapon == 30 || byteWeapon == 31) fFix = 0.028f;
                            else if (byteWeapon == 33) fFix = 0.01897f;

                            float fDistX = std::sqrt(vecVector.x * vecVector.x + vecVector.y * vecVector.y);

                            if (vecVector.x <= 0.0 && vecVector.y >= 0.0 || vecVector.x >= 0.0 && vecVector.y >= 0.0)
                                fVecX = (std::acosf(vecVector.x / fDistX) + fFix) - TheCamera.m_aCams[0].m_fHorizontalAngle;
                            if (vecVector.x >= 0.0 && vecVector.y <= 0.0 || vecVector.x <= 0.0 && vecVector.y <= 0.0)
                                fVecX = (-std::acosf(vecVector.x / fDistX) + fFix) - TheCamera.m_aCams[0].m_fHorizontalAngle;

                            if (const float fSmoothX = fVecX / (gConfig->data["crosshair"]["smoothAim"]["smooth"].get<float>() * 2);
                                fSmoothX > -1.0 && fSmoothX < 0.5 && fVecX > -2.0 && fVecX < 2.0)
                                TheCamera.m_aCams[0].m_fHorizontalAngle += fSmoothX;
                        }
                    }
                }
            }
        }
    }
}

void lpCrosshair::noSpread() {
    float spread;
    if (gConfig->data["crosshair"]["noSpread"]["state"].get<bool>())
        spread = gConfig->data["crosshair"]["noSpread"]["spread"].get<float>();
    else
        spread = 100.f;
    float actual_spread = (spread <= 0) ? 0.1f : spread;
    float spread_for_non_shotguns = 0.75f * (spread / 100.0f);
    float spread_for_shotguns = 0.050000001f * (actual_spread / 100.0f);

    DWORD* ptr_spread_non_shotguns = reinterpret_cast<DWORD*>(&spread_for_non_shotguns);
    DWORD* ptr_spread_shotguns = reinterpret_cast<DWORD*>(&spread_for_shotguns);

    *(DWORD*)0x8D6110 = *ptr_spread_non_shotguns;
    *(DWORD*)0x8D611C = *ptr_spread_shotguns;
}

//std::pair<int, sampapi::v037r3::CPed*> getNearCharToCenter(float radius, float centerX, float centerY, float maxDistance) {
//    std::vector<playerDistance> players;
//
//    for (int i = 0; i < sampapi::v037r3::RefNetGame()->GetPlayerPool()->MAX_PLAYERS; i++) {
//        if (sampapi::v037r3::RefNetGame()->GetPlayerPool()->IsConnected(i)) {
//            sampapi::v037r3::CPed* pPed = sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(i)->m_pPed;
//            if (pPed && pPed != nullptr && !pPed->IsDead()) {
//                sampapi::CVector position = sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(i)->m_onfootData.m_position;
//
//                if (utils->getDistanceBetweenCoords3d(position.x, position.y, position.z, FindPlayerCoors(-1).x, FindPlayerCoors(-1).y, FindPlayerCoors(-1).z) <= maxDistance) {
//                    sampapi::CVector convertedPosition;
//                    utils->CalcScreenCoords(&position, &convertedPosition);
//
//                    if (convertedPosition.z > 1.f && sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(i)->m_pPed->IsOnScreen()) {
//                        float distance = utils->getDistanceBetweenCoords2d(centerX, centerY, convertedPosition.x, convertedPosition.y);
//
//                        if (distance <= radius)
//                            players.push_back({ distance, i, pPed });
//                    }
//                }
//            }
//        }
//    }
//
//    if (!players.empty()) {
//        std::sort(players.begin(), players.end(), [](playerDistance a, playerDistance b) {
//            return a.distance < b.distance;
//            });
//        return std::pair<int, sampapi::v037r3::CPed*>(players.front().playerId, players.front().ped);
//    }
//
//    return std::pair<int, sampapi::v037r3::CPed*>(-1, nullptr);
//}
//
//int getNearBone(sampapi::v037r3::CPed* ped, float centerX, float centerY) {
//    std::vector<boneDistance> bones;
//
//    for (int bone : iBoneList) {
//        sampapi::CVector position;
//        sampapi::CVector convertedPosition;
//        ped->GetBonePosition(bone, &position);
//        utils->CalcScreenCoords(&position, &convertedPosition);
//
//        if (convertedPosition.z > 1.f) {
//            float distance = utils->getDistanceBetweenCoords2d(centerX, centerY, convertedPosition.x, convertedPosition.y);
//            bones.push_back({ distance, bone });
//        }
//    }
//
//    if (!bones.empty()) {
//        std::sort(bones.begin(), bones.end(), [](boneDistance a, boneDistance b) {
//            return a.distance < b.distance;
//            });
//        return bones.front().bone;
//    }
//
//    return -1;
//}