#ifndef LPPLAYER_FILE_H
#define LPPLAYER_FILE_H

#include <common.h>

class lpPlayer {
public:
	void callFunctions();
private:
	void airBrake();
	void infinityRun();
	void infinityOxygen();
	void fastRotation();
	void megaJump();
	void noFall();
	void fastRespawn();
	void sbiv();

	void destroyAnimsInstantly();
	bool isPlayingAnimation(CEntity* pEntity, char const* szAnimName);
};

extern lpPlayer* gPlayer;

#endif