#include "rakhookInstall.h"

#include <d3d9.h>
#include <random>

#include <RakHook/rakhook.hpp>
#include <RakNet/PacketEnumerations.h>

#include <sampapi/Synchronization.h>
#include <sampapi/CChat.h>
#include <sampapi/CNetGame.h>

#include <CCamera.h>
#include <CPools.h>

#include "plugin/cheats/utils/lpUtils.h"
#include "config/config.h"
#include "sampver/sampver.h"
#include "main.h"

struct damage {
    int id;
    float damage;
    BYTE weapon;
    int bone;
} n_damage;

rakhookInstall* gRakhook;

rakhookInstall::rakhookInstall() {
    rakhook::on_receive_packet += [](Packet* p) -> bool {
        if (+(*p->data) == ID_BULLET_SYNC) {
            RakNet::BitStream bs(p->data, p->length, false);

            if (gConfig->data["player"]["antistun"]["state"].get<bool>()) {
                bs.ResetReadPointer();
                bs.IgnoreBits(8);

                uint16_t playerId;
                bs.Read(playerId);

                if (gSampVersion->getSampVersion() == samp_ver::v037r31) {
                    sampapi::v037r3::Synchronization::BulletData bullet = { 0 };
                    bs.Read((char*)&bullet, sizeof(sampapi::v037r3::Synchronization::BulletData));

                    if (bullet.m_nTargetId == sampapi::v037r3::RefNetGame()->GetPlayerPool()->m_localInfo.m_nId) {
                        srand((unsigned)time(NULL));

                        bool chance = rand() % 100 >= gConfig->data["player"]["antistun"]["chance"].get<float>() ? true : false;

                        if (!gConfig->data["player"]["antistun"]["activateByKey"].get<bool>())
                            return chance;
                        else
                            if (utils->isKeyDown(utils->getVirtualKey(gConfig->data["player"]["antistun"]["button"].get<ImGuiKey>())))
                                return chance;
                    }
                }
                else {
                    sampapi::v037r1::Synchronization::BulletData bullet = { 0 };
                    bs.Read((char*)&bullet, sizeof(sampapi::v037r1::Synchronization::BulletData));

                    if (bullet.m_nTargetId == sampapi::v037r1::RefNetGame()->GetPlayerPool()->m_localInfo.m_nId) {
                        srand((unsigned)time(NULL));

                        bool chance = rand() % 100 >= gConfig->data["player"]["antistun"]["chance"].get<float>() ? true : false;

                        if (!gConfig->data["player"]["antistun"]["activateByKey"].get<bool>())
                            return chance;
                        else
                            if (utils->isKeyDown(utils->getVirtualKey(gConfig->data["player"]["antistun"]["button"].get<ImGuiKey>())))
                                return chance;
                    }
                }
            }
        }
        return true;
    };

    rakhook::on_send_packet += [](RakNet::BitStream* bs, PacketPriority& priority, PacketReliability& reliability, char& ord_channel) -> bool {
        if (+(*bs->GetData()) == ID_PLAYER_SYNC) {
            if (gConfig->data["player"]["antiBunnyHop"].get<bool>()) {
                sampapi::v037r3::Synchronization::OnfootData data;
                memset(&data, 0, sizeof(sampapi::v037r3::Synchronization::OnfootData));

                bs->ResetReadPointer();
                bs->IgnoreBits(8);
                bs->Read((PCHAR)&data, sizeof(sampapi::v037r3::Synchronization::OnfootData));

                if (data.m_controllerState.m_bButtonSquare > 0 && data.m_controllerState.m_bButtonCross > 0)
                    data.m_controllerState.m_bButtonCross = 0;
                if (data.m_nSpecialAction == 40)
                    data.m_nSpecialAction = 32;
                if (data.m_animation.m_nId == 1198)
                    data.m_animation.m_nId = 1195;

                bs->ResetWritePointer();
                bs->Write((byte)ID_PLAYER_SYNC);
                bs->Write((PCHAR)&data, sizeof(sampapi::v037r3::Synchronization::OnfootData));
            }
        }
        else if (+(*bs->GetData()) == ID_BULLET_SYNC) {
            /*if (gConfig->data["crosshair"]["silentAim"]["state"].get<bool>()) {
                static CVector2D vecCrosshair;

                if (FindPlayerPed()->m_nWeaponModelId != 34) {
                    vecCrosshair.x = variables.displaySize[0] * 0.53f;
                    vecCrosshair.y = variables.displaySize[1] * 0.4f;
                }
                else {
                    vecCrosshair.x = variables.displaySize[0] / 2.0f;
                    vecCrosshair.y = variables.displaySize[1] / 2.0f;
                }

                bs->ResetReadPointer();
                bs->IgnoreBits(8);

                if (gSampVersion->getSampVersion() == samp_ver::v037r31) {
                    const BYTE byteWeapon = sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetLocalPlayer()->m_weaponsData.m_nCurrentWeapon;
                    if (byteWeapon > 21 && byteWeapon < 34 || byteWeapon == 38) {
                        std::pair<int, sampapi::v037r3::CPed*> nearId = r3->getNearCharToCenter(gConfig->data["crosshair"]["FOV"]["radius"].get<float>(), vecCrosshair.x, vecCrosshair.y, gConfig->data["crosshair"]["silentAim"]["maxDistance"].get<float>());
                        if (nearId.first != -1) {
                            int nearBone = r3->getNearBone(nearId.second, vecCrosshair.x, vecCrosshair.y);
                            if (nearBone != -1) {
                                srand((unsigned)time(NULL));

                                sampapi::v037r3::Synchronization::BulletData bullet = { 0 };
                                bs->Read((char*)&bullet, sizeof(sampapi::v037r3::Synchronization::BulletData));

                                sampapi::CVector bonePosition;
                                sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetPlayer(nearId.first)->m_pPed->GetBonePosition(nearBone, &bonePosition);

                                bullet.m_nTargetType = 1;
                                bullet.m_nTargetId = nearId.first;
                                bullet.m_target = bonePosition;

                                static const std::unordered_map<int, double> damage = {
                                    {22, 8.25}, {23, 13.2}, {24, 46.200000762939},
                                    {25, 30}, {26, 30}, {27, 30}, {28, 6.6},
                                    {29, 8.25}, {30, 9.9}, {31, 9.9000005722046},
                                    {32, 6.6}, {33, 25}, {38, 46.2}
                                };

                                static std::mt19937 gen(std::random_device{}());
                                static std::uniform_real_distribution<> dis(0.0, 1e-6);

                                auto it = damage.find(byteWeapon);
                                float resultDamage = static_cast<float>((it != damage.end() ? it->second : 0.0) + dis(gen));

                                bs->ResetWritePointer();
                                bs->Write((byte)ID_BULLET_SYNC);
                                bs->Write((char*)&bullet, sizeof(sampapi::v037r3::Synchronization::BulletData));

                                sampapi::v037r3::RefChat()->AddMessage(-1, ("id: " + std::to_string(nearId.first) + " | Damage: " + std::to_string(resultDamage)).c_str());
                                sampapi::v037r3::RefNetGame()->GetPlayerPool()->GetLocalPlayer()->SendGiveDamage(nearId.first, resultDamage, byteWeapon, nearBone);
                            }
                        }
                    }
                }
                else if (gSampVersion->getSampVersion() == samp_ver::v037r1) {
                    const BYTE byteWeapon = sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetLocalPlayer()->m_weaponsData.m_nCurrentWeapon;
                    if (byteWeapon > 21 && byteWeapon < 34 || byteWeapon == 38) {
                        std::pair<int, sampapi::v037r1::CPed*> nearId = r1->getNearCharToCenter(gConfig->data["crosshair"]["FOV"]["radius"].get<float>(), vecCrosshair.x, vecCrosshair.y, gConfig->data["crosshair"]["silentAim"]["maxDistance"].get<float>());
                        if (nearId.first != -1) {
                            int nearBone = r1->getNearBone(nearId.second, vecCrosshair.x, vecCrosshair.y);
                            if (nearBone != -1) {
                                srand((unsigned)time(NULL));

                                sampapi::v037r1::Synchronization::BulletData bullet = { 0 };
                                bs->Read((char*)&bullet, sizeof(sampapi::v037r1::Synchronization::BulletData));

                                sampapi::CVector bonePosition;
                                sampapi::v037r1::RefNetGame()->GetPlayerPool()->GetPlayer(nearId.first)->m_pPed->GetBonePosition(nearBone, &bonePosition);

                                bullet.m_nTargetType = 1;
                                bullet.m_nTargetId = nearId.first;
                                bullet.m_target = bullet.m_center = bonePosition;

                                static const std::unordered_map<int, double> damage = {
                                    {22, 8.25}, {23, 13.2}, {24, 46.200000762939},
                                    {25, 30}, {26, 30}, {27, 30}, {28, 6.6},
                                    {29, 8.25}, {30, 9.9}, {31, 9.9000005722046},
                                    {32, 6.6}, {33, 25}, {38, 46.2}
                                };

                                static std::mt19937 gen(std::random_device{}());
                                static std::uniform_real_distribution<> dis(0.0, 1e-6);

                                auto it = damage.find(byteWeapon);
                                float resultDamage = static_cast<float>((it != damage.end() ? it->second : 0.0) + dis(gen));

                                bs->ResetWritePointer();
                                bs->Write((byte)ID_BULLET_SYNC);
                                bs->Write((char*)&bullet, sizeof(sampapi::v037r1::Synchronization::BulletData));

                                sampapi::v037r1::RefChat()->AddMessage(-1, ("id: " + std::to_string(nearId.first) + " | Damage: " + std::to_string(resultDamage) + " | Channel: " + std::to_string(+ord_channel)).c_str());

                                n_damage.id = nearId.first;
                                n_damage.damage = resultDamage;
                                n_damage.weapon = byteWeapon;
                                n_damage.bone = nearBone;

                                // damage
                                variables.tasklist.add_task([nearId, resultDamage, byteWeapon, nearBone]() -> ktwait {
                                    co_await std::chrono::milliseconds(1);

                                    RakNet::BitStream bsDamage;

                                    bsDamage.Write(false);
                                    bsDamage.Write((uint16_t)n_damage.id);
                                    bsDamage.Write(n_damage.damage);
                                    bsDamage.Write((uint32_t)n_damage.weapon);
                                    bsDamage.Write((uint32_t)n_damage.bone);

                                    rakhook::send_rpc(115, &bsDamage, PacketPriority::HIGH_PRIORITY, PacketReliability::RELIABLE_ORDERED, 0, false);

                                    memset(&n_damage, 0, sizeof(n_damage));
                                });
                            }
                        }
                    }
                }
            }*/
        }
        return true;
    };

    rakhook::on_receive_rpc += [](unsigned char& id, RakNet::BitStream* bs) -> bool {
        if (gConfig->data["player"]["antiFreeze"].get<bool>() && id == 15)
            return false;

        return true;
    };

    rakhook::on_send_rpc += [](int& id, RakNet::BitStream* bs, PacketPriority& priority, PacketReliability& reliability, char& ord_channel, bool& sh_timestamp) -> bool {
        if (id == 50) {
            bs->ResetReadPointer();
            
            uint32_t length;
            bs->Read(length);

            std::string message(length, '\0');
            bs->Read(message.data(), length);

            if (message == "/sw") {
                variables.is_open_menu ^= true;
                return false;
            }
        }

        return true;
    };
}