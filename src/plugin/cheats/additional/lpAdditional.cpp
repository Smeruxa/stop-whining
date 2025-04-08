#include "lpAdditional.h"
#include "config/config.h"

#include <common.h>
#include <CCamera.h>

lpAdditional* gAdditional;

void lpAdditional::callFunctions() {
	FOV();
}

void lpAdditional::FOV() {
	TheCamera.LerpFOV(gConfig->data["additional"]["FOV"]["state"].get<bool>() ? gConfig->data["additional"]["FOV"]["num"].get<float>() : 70.f, 
						gConfig->data["additional"]["FOV"]["state"].get<bool>() ? gConfig->data["additional"]["FOV"]["num"].get<float>() : 70.f, 
						999988888.f, true);
}