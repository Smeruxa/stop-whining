#ifndef LPCROSSHAIR_FILE_H
#define LPCROSSHAIR_FILE_H

#include <common.h>
#include <kthook/kthook.hpp>

class lpCrosshair {
public:
	lpCrosshair();
	void callFunctions();

	static void __fastcall DoBulletImpactHooked(void* weapon, void* EDX, CEntity* owner, CEntity* victim, CVector* startPoint, CVector* endPoint, CColPoint* colPoint, int arg5);
	typedef void(__fastcall* DoBulletImpact)(void*, void*, CEntity*, CEntity*, CVector*, CVector*, CColPoint*, int);

	static bool __fastcall FireInstantHitHooked(void* _this, void* EDX, CEntity* firingEntity, CVector* origin, CVector* muzzlePosn, CEntity* targetEntity, CVector* target, CVector* originForDriveBy, bool arg6, bool muzzle);
	typedef bool(__fastcall* FireInstantHit)(void*, void*, CEntity*, CVector*, CVector*, CEntity*, CVector*, CVector*, bool, bool);
private:
	void noSpread();
	void smoothAim();
	void triggerBot();
	void silentAim();
};

extern lpCrosshair* gCrosshair;

#endif