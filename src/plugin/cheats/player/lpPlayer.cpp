#include "lpPlayer.h"
#include "config/config.h"
#include "plugin/cheats/utils/lpUtils.h"

#include <CWorld.h>
#include <CEntity.h>
#include <CCamera.h>
#include <CShadows.h>
#include <RenderWare.h>
#include <CCamera.h>

lpPlayer* gPlayer;

void lpPlayer::callFunctions() {
	if (!FindPlayerPed()->IsAlive())
		return;

	// get current cam - TheCamera.m_aCams[0]

	airBrake();
	infinityRun();
	infinityOxygen();
	fastRotation();
	megaJump();
	noFall();
	fastRespawn();
	sbiv();
}

void lpPlayer::sbiv() {
	if (gConfig->data["player"]["sbiv"]["state"].get<bool>() && utils->isKeyPressed(utils->getVirtualKey(gConfig->data["player"]["sbiv"]["button"].get<ImGuiKey>())))
		destroyAnimsInstantly();
}

void lpPlayer::fastRespawn() {
	CPlayerPed* pedself = FindPlayerPed();
	if (gConfig->data["player"]["fastRespawn"].get<bool>() && pedself->m_fHealth == 0.f)
		pedself->m_fHealth = 99.f;
}

void lpPlayer::noFall() {
	if (gConfig->data["player"]["noFall"].get<bool>()) {
		bool bOutResult = false; 
		CEntity* pOutEntity;

		CPlayerPed* pedself = FindPlayerPed();
		if (isPlayingAnimation(pedself, "FALL_FALL") && pedself->GetPosition().z - CWorld::FindGroundZFor3DCoord(pedself->GetPosition().x, pedself->GetPosition().y, pedself->GetPosition().z, &bOutResult, &pOutEntity) < 1.5f) {
			destroyAnimsInstantly();
			pedself->m_vecMoveSpeed.x = pedself->m_vecMoveSpeed.y = pedself->m_vecMoveSpeed.z = 0.f;
		}
	}
}

void lpPlayer::destroyAnimsInstantly() {
	FindPlayerPed()->m_pIntelligence->FlushImmediately(true);
}

void lpPlayer::megaJump() {
	*(float*)0x8703C0 = (gConfig->data["player"]["megaJump"]["state"].get<bool>()) ? (8.5f * gConfig->data["player"]["megaJump"]["increase"].get<float>()) : 8.5f;
}

void lpPlayer::fastRotation() {
	CPlayerPed* pedself = FindPlayerPed();
	if (gConfig->data["player"]["fastRotation"]["state"].get<bool>())
		pedself->m_fHeadingChangeRate = gConfig->data["player"]["fastRotation"]["speed"].get<float>();
	else
		pedself->RestoreHeadingRate();
}

void lpPlayer::infinityOxygen() {
	if (gConfig->data["player"]["infinityOxygen"].get<bool>()) {
		CPlayerPed* pedself = FindPlayerPed();
		if (pedself != nullptr && pedself->m_pPlayerData->m_fBreath < 100.f)
			pedself->ResetPlayerBreath();
	}
}

void lpPlayer::infinityRun() {
	if (gConfig->data["player"]["infinityRun"].get<bool>()) {
		CPlayerPed* pedself = FindPlayerPed();
		if (pedself != nullptr && pedself->m_pPlayerData->m_fTimeCanRun < 100.f)
			pedself->ResetSprintEnergy();
	}
}

void lpPlayer::airBrake() {
	static bool airbrakeState = false;
	static CVector position = { 0.f, 0.f, 0.f };

	if (gConfig->data["player"]["airbrake"]["state"].get<bool>()) {
		if (utils->isKeyPressed(utils->getVirtualKey(gConfig->data["player"]["airbrake"]["button"].get<ImGuiKey>()))) {
			airbrakeState ^= true;
			if (airbrakeState) {
				position = FindPlayerPed()->m_matrix->pos;
				if (FindPlayerVehicle(-1, false))
					position = FindPlayerVehicle(-1, false)->m_matrix->pos;
			}
		}

		if (airbrakeState) {
			float speed = gConfig->data["player"]["airbrake"]["speed"];

			float fCameraRotation = *(float*)0xB6F178;
			CPlayerPed* pPedSelf = FindPlayerPed();

			pPedSelf->m_fCurrentRotation = pPedSelf->m_fAimingRotation = -fCameraRotation;
			pPedSelf->m_vecMoveSpeed.z = 0.f;

			CVehicle* pVehicle = FindPlayerVehicle(-1, false);
			if (pVehicle) {
				pVehicle->m_matrix->SetRotateZOnly(-fCameraRotation);
				pVehicle->m_vecMoveSpeed.z = 0.1f;
			}
			else
				pPedSelf->m_nPedFlags.bIsStanding = pPedSelf->m_nPedFlags.bWasStanding = pPedSelf->m_nPedFlags.bStayInSamePlace = true;

			if (utils->isKeyDown(87)) // W
				position.x += sinf(fCameraRotation) * speed, position.y += cosf(fCameraRotation) * speed;
			if (utils->isKeyDown(83)) // S
				position.x -= sinf(fCameraRotation) * speed, position.y -= cosf(fCameraRotation) * speed;
			if (utils->isKeyDown(68)) // D
				position.x += cosf(fCameraRotation) * speed, position.y -= sinf(fCameraRotation) * speed;
			if (utils->isKeyDown(65)) // A
				position.x -= cosf(fCameraRotation) * speed, position.y += sinf(fCameraRotation) * speed;

			/*if (GetAsyncKeyState(VK_SPACE) & 0x8000)
				position.z += speed;
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
				position.z -= speed;*/

			if (utils->isKeyDown(VK_SPACE))
				position.z += speed;
			if (utils->isKeyDown(VK_CONTROL))
				position.z -= speed;

			if (pVehicle)
				pVehicle->m_matrix->pos = position;
			else
				pPedSelf->m_matrix->pos = position;
		}
	}
	else
		airbrakeState = false;
}

bool lpPlayer::isPlayingAnimation(CEntity* pEntity, char const* szAnimName) {
	return RpAnimBlendClumpGetAssociation(pEntity->m_pRwClump, szAnimName);
}