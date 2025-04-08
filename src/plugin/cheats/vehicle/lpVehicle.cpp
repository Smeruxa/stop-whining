#include "lpVehicle.h"

#include "plugin/cheats/utils/lpUtils.h"
#include "config/config.h"

#include <CTask.h>
#include <CPools.h>
#include <sampapi/CChat.h>
#include <ePedBones.h>

#include <FxManager_c.h>
#include <Fx_c.h>

lpVehicle* gVehicle;

void lpVehicle::callFunctions() {
	CVehicle* vehicle = FindPlayerVehicle(-1, false);
	if (vehicle && vehicle != nullptr) {
		flip(vehicle);
		noBike(vehicle);
		godmode(vehicle);
		fastExit(vehicle);
		speedhack(vehicle);
		drift(vehicle);
		jumpCar(vehicle);
		

		// not work - Drift + Flip

		*reinterpret_cast<uint8_t*>(0x6C2759) = static_cast<int>(gConfig->data["vehicle"]["underwaterDrive"]["state"].get<bool>()); // underwater
		*reinterpret_cast<uint32_t*>(9867602) = static_cast<int>(gConfig->data["vehicle"]["overwaterDrive"]["state"].get<bool>()); // overwater
		*reinterpret_cast<uint8_t*>(0x969161) = static_cast<int>(gConfig->data["vehicle"]["bigJumpBMX"]["state"].get<bool>()); // bigJumpBMX
	}
}

void lpVehicle::drift(CVehicle* vehicle) {
	if (gConfig->data["vehicle"]["drift"]["state"].get<bool>() && utils->isKeyDown(utils->getVirtualKey(gConfig->data["vehicle"]["drift"]["button"].get<ImGuiKey>()))) {//utils->isKeyDown(utils->getVirtualKey(gConfig->data["vehicle"]["drift"]["button"].get<ImGuiKey>()))) {
		if (utils->isKeyDown(utils->getVirtualKey(ImGuiKey_A)))
			vehicle->m_vecTurnSpeed.z += 0.002f;
		else if (utils->isKeyDown(utils->getVirtualKey(ImGuiKey_D)))
			vehicle->m_vecTurnSpeed.z -= 0.002f;
	}
}

void lpVehicle::jumpCar(CVehicle* vehicle) {
	if (gConfig->data["vehicle"]["jumpCar"]["state"].get<bool>() && utils->isKeyPressed(utils->getVirtualKey(gConfig->data["vehicle"]["jumpCar"]["button"].get<ImGuiKey>())))
		vehicle->m_vecMoveSpeed.z += gConfig->data["vehicle"]["jumpCar"]["zPosition"].get<float>() / 10.f;
}

void lpVehicle::speedhack(CVehicle* vehicle) {
	if ((GetAsyncKeyState(VK_LMENU) & 0x8000) && gConfig->data["vehicle"]["speedhack"]["state"].get<bool>()) {
		CVector speed = vehicle->m_vecMoveSpeed;
		float heading = *(float*)0x441DB0;
		float turbo = *reinterpret_cast<float*>(reinterpret_cast<uint32_t*>(0xB7CB5C)) / 85.0f;
		float xforce = turbo, yforce = turbo, zforce = turbo;

		float Sin = static_cast<float>(sin(-heading * 3.14159265358979323846 / 180.0f));
		float Cos = static_cast<float>(cos(-heading * 3.14159265358979323846 / 180.0f));

		if (speed.x > -0.01f && speed.x < 0.01f) xforce = 0.0f;
		if (speed.y > -0.01f && speed.y < 0.01f) yforce = 0.0f;
		if (speed.z < 0) zforce = -zforce;
		if (speed.z > -2.0f && speed.z < 15.0f) zforce = 0.0f;
		if (Sin > 0 && speed.x < 0) xforce = -xforce;
		if (Sin < 0 && speed.x > 0) xforce = -xforce;
		if (Cos > 0 && speed.y < 0) yforce = -yforce;
		if (Cos < 0 && speed.y > 0) yforce = -yforce;

		float accuracy = gConfig->data["vehicle"]["speedhack"]["accuracy"].get<float>();
		vehicle->m_vecMoveSpeed += CVector(xforce * Sin * accuracy, yforce * Cos * accuracy, zforce / 2);
	}
}

void lpVehicle::fastExit(CVehicle* vehicle) {
	if (gConfig->data["vehicle"]["fastExit"]["state"].get<bool>() && utils->isKeyPressed(utils->getVirtualKey(gConfig->data["vehicle"]["fastExit"]["button"].get<ImGuiKey>()))) {
		CPed* ped = FindPlayerPed();

		ped->m_pIntelligence->SetTaskDuckSecondary(TASK_COMPLEX_CAR_QUICK_BE_DRAGGED_OUT);
		ped->Teleport(CVector(ped->m_matrix->pos.x, ped->m_matrix->pos.y, ped->m_matrix->pos.z + 1.5f), false);
	}
}

void lpVehicle::godmode(CVehicle* vehicle) {
	if (gConfig->data["vehicle"]["godmode"]["state"].get<bool>()) {
		vehicle->Fix();
		if (gConfig->data["vehicle"]["godmode"]["constHp"].get<bool>())
			vehicle->m_fHealth = 1000.f;
	}
}

void lpVehicle::flip(CVehicle* vehicle) {
	if (gConfig->data["vehicle"]["flip"]["state"].get<bool>() && utils->isKeyPressed(utils->getVirtualKey(gConfig->data["vehicle"]["flip"]["button"].get<ImGuiKey>())))
		vehicle->Teleport(vehicle->m_matrix->pos, true);
}

void lpVehicle::noBike(CVehicle* vehicle) {
	CPed* ped = FindPlayerPed();
	if (gConfig->data["vehicle"]["noBike"]["state"].get<bool>()) {
		if (!gConfig->data["vehicle"]["noBike"]["turnOffInWater"].get<bool>())
			ped->m_nPedFlags.CantBeKnockedOffBike = 1;
		else
			ped->m_nPedFlags.CantBeKnockedOffBike = ((vehicle->m_nPhysicalFlags.bSubmergedInWater == 1) ? 0 : 1);
	}
}