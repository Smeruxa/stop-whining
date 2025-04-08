#ifndef LPVEHICLE_FILE_H
#define LPVEHICLE_FILE_H

#include <common.h>

class lpVehicle {
public:
	void callFunctions();
private:
	void flip(CVehicle* vehicle);
	void noBike(CVehicle* vehicle);
	void godmode(CVehicle* vehicle);
	void fastExit(CVehicle* vehicle);
	void speedhack(CVehicle* vehicle);
	void jumpCar(CVehicle* vehicle);
	void drift(CVehicle* vehicle);
};

extern lpVehicle* gVehicle;

#endif