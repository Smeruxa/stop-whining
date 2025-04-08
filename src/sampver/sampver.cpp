#include "sampver.h"
#include <Windows.h>

sampver* gSampVersion;

samp_ver sampver::getSampVersion() {
	static bool init = false;
	static samp_ver v = samp_ver::unknown;

	if (!init) {
		std::uintptr_t base = reinterpret_cast<std::uintptr_t>(GetModuleHandle("samp.dll"));

		auto* ntheader = reinterpret_cast<IMAGE_NT_HEADERS*>(base + reinterpret_cast<IMAGE_DOS_HEADER*>(base)->e_lfanew);
		std::uintptr_t ep = ntheader->OptionalHeader.AddressOfEntryPoint;
		switch (ep) {
			case 0x31DF13: v = samp_ver::v037r1; break;
			case 0xCC4D0: v = samp_ver::v037r31; break;
			case 0xCBCB0: v = samp_ver::v037r4; break;
			case 0xFDB60: v = samp_ver::v03dlr1; break;
			default: break;
		}

		init = true;
	}

	return v;
}