#include<Windows.h>
#include<TlHelp32.h>
#include <iostream>
#include "../Offsets.h"

uintptr_t moduleBase;
uintptr_t engineBase;
DWORD procId;
HWND hwnd;
HANDLE hProcess;

uintptr_t GetModuleBaseAddress(const char* modName) {
	HANDLE hSnap =
		CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap != INVALID_HANDLE_VALUE) {
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry)) {
			do {
				if (!strcmp(modEntry.szModule, modName)) {
					CloseHandle(hSnap);
					return (uintptr_t)modEntry.modBaseAddr;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
}

template <typename T>

T RPM(SIZE_T address) {
	T buffer;
	ReadProcessMemory(hProcess, (LPCVOID)address, &buffer, sizeof(T), NULL);
	return buffer;
}

template<typename T> void WPM(SIZE_T address, T buffer) {
	WriteProcessMemory(hProcess, (LPVOID)address, &buffer, sizeof(buffer), NULL);
}

uintptr_t getLocalPlayer() {
	return RPM<uintptr_t>(moduleBase + dwLocalPlayer);
}

uintptr_t getPlayer(int index) {
	return RPM<uintptr_t>(moduleBase + dwEntityList + index * 0x10);
}

struct Vect3 {
	float x;
	float y;
	float z;

	Vect3 operator+(Vect3 d) {
		return { x + d.x,y + d.y,z + d.z };
	}

	Vect3 operator-(Vect3 d) {
		return { x - d.x,y - d.y,z - d.z };
	}

	Vect3 operator*(float d) {
		return { x * d,y * d,z * d };
	}

	void Nomalize() {
		printf("%d %d %d\n", x, y, z);
		while (y < -180) y += 360;
		while (y > 180)y -= 360;
		if (x > 89)x = 89;
		if (x < -89)x = -89;
	}
};

uintptr_t getClientState() {
	return RPM<uintptr_t>(engineBase + dwClientState);
}

Vect3 getViewAngles() {
	return RPM<Vect3>(engineBase + dwClientState + dwClientState_ViewAngles);
}

int getShotFired() {
	return RPM<int>(moduleBase + dwLocalPlayer + m_iShotsFired);
}

Vect3 getAimPunchAngles() {
	return RPM<Vect3>(moduleBase + dwLocalPlayer + m_aimPunchAngle);
}

int main() {
	hwnd = FindWindowA(NULL, "Counter-Strike: Global Offensive");
	GetWindowThreadProcessId(hwnd, &procId);
	moduleBase = GetModuleBaseAddress("client.dll");
	engineBase = GetModuleBaseAddress("engine.dll");
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);

	uintptr_t player = getLocalPlayer();
	Vect3 aimPunchAngle = getAimPunchAngles();

	Vect3 oPunch{ 0,0,0 };

	while (!GetAsyncKeyState(VK_END)) {
		uintptr_t localPlayer = getLocalPlayer();
		Vect3 viewAngles = getViewAngles();
		int iShotFired = getShotFired();
		Vect3 tempAngle = { 0,0,0 };
		Vect3 aimPunchAngle = getAimPunchAngles();

		if (iShotFired >= 3) {
			tempAngle = viewAngles + oPunch - (aimPunchAngle * 2);
			tempAngle.Nomalize();
			oPunch = aimPunchAngle * 2;
			WPM<Vect3>(engineBase + dwClientState + dwClientState_ViewAngles, tempAngle);
		}
		else {
			oPunch = { 0,0,0 };
		}
	}
	return 0;

}