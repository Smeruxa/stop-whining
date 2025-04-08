#ifndef LPUTILS_FILE_H
#define LPUTILS_FILE_H

#include <unordered_set>
#include <Windows.h>
#include <imgui.h>
#include <common.h>
#include <sampapi/CVector.h>
#include <sampapi/CNetGame.h>

class lpUtils {
public:
	struct boneDistance {
		float distance;
		int bone;
	};

	class r3 {
	public:
		static std::pair<int, sampapi::v037r3::CPed*> getNearCharToCenter(float radius, float centerX, float centerY, float maxDistance);
		static int getNearBone(sampapi::v037r3::CPed* ped, float centerX, float centerY);

		struct playerDistance {
			float distance;
			int playerId;
			sampapi::v037r3::CPed* ped;
		};
	};

	class r1 {
	public:
		static std::pair<int, sampapi::v037r1::CPed*> getNearCharToCenter(float radius, float centerX, float centerY, float maxDistance);
		static int getNearBone(sampapi::v037r1::CPed* ped, float centerX, float centerY);

		struct playerDistance {
			float distance;
			int playerId;
			sampapi::v037r1::CPed* ped;
		};
	};

	bool isKeyDown(int key);
	bool isKeyPressed(int virtualKeyCode);

	void CalcScreenCoords(CVector* vecWorld, CVector* vecScreen);
	void CalcScreenCoords(sampapi::CVector* vecWorld, sampapi::CVector* vecScreen);
	void getCrosshairPosition(CVector* vec_out);
	void setCrosshairState(bool param);

	float getDistanceBetweenCoords3d(float x, float y, float z, float x1, float y1, float z1);
	float getDistanceBetweenCoords2d(float x, float y, float x1, float y1);

	void SendKeyInput(WPARAM keyCode);

    int getVirtualKey(ImGuiKey key);

	std::unordered_set<int> downKeys;
private:
	struct _Keys {
		bool        bPressed;
		DWORD       dwStartTime;
	} kPressingKeys[256];
};

extern lpUtils::r3* r3;
extern lpUtils::r1* r1;
extern lpUtils* utils;

#endif 