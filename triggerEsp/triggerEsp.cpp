#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include "../Offsets.h"

#pragma comment(lib,"winmm.lib")

const int SCREEN_WIDTH = GetSystemMetrics(SM_CXSCREEN);
const int SCREEN_HEIGHT = GetSystemMetrics(SM_CYSCREEN);

uintptr_t moduleBase;
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

int getTeam(uintptr_t player) {
	return RPM<int>(player + m_iTeamNum);
}

int getCrosshairID(uintptr_t player) {
	return RPM<int>(player + m_iCrosshairId);
}

struct glowStructEnemy {
	float red = 1.f;
	float green = 0.f;
	float blue = 0.f;
	float alpha = 0.6f;
	uint8_t padding[8];
	float unknown = 1.f;
	uint8_t padding2[4];
	BYTE renderOccluded = true;
	BYTE renderUnoccluded = true;
	BYTE fullBloom = false;
} glowEnm;

int main() {
	hwnd = FindWindowA(NULL, "Counter-Strike: Global Offensive");
	GetWindowThreadProcessId(hwnd, &procId);
	moduleBase = GetModuleBaseAddress("client.dll");
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);
	BOOL radarOn = FALSE, espOn = FALSE, triggerOn = FALSE, silentAimOn = FALSE;
	printf("Trigger: %s\nEsp: %s\nRadar: %s\nSilent Aim: %s\n", triggerOn ? "On" : "Off", espOn ? "On" : "Off", radarOn ? "On" : "Off", silentAimOn ? "On" : "Off");


	while (!GetAsyncKeyState(VK_END)) {
		int CrosshairId = getCrosshairID(getLocalPlayer());
		int CrosshairTeam = getTeam(getPlayer(CrosshairId - 1));
		int LocalTeam = getTeam(getLocalPlayer());

		if (GetAsyncKeyState(VK_F6) & 1) {
			triggerOn = !triggerOn;
			system("cls");
			printf("Trigger: %s\nEsp: %s\nRadar: %s\nSilent Aim: %s\n", triggerOn ? "On" : "Off", espOn ? "On" : "Off", radarOn ? "On" : "Off", silentAimOn ? "On" : "Off");
			PlaySound("..\\click.wav", NULL, SND_FILENAME | SND_SYNC);
		}
		if (GetAsyncKeyState(VK_F7) & 1) {
			espOn = !espOn;
			system("cls");
			printf("Trigger: %s\nEsp: %s\nRadar: %s\nSilent Aim: %s\n", triggerOn ? "On" : "Off", espOn ? "On" : "Off", radarOn ? "On" : "Off", silentAimOn ? "On" : "Off");
			PlaySound("..\\click.wav", NULL, SND_FILENAME | SND_SYNC);
		}
		if (GetAsyncKeyState(VK_F8) & 1) {
			radarOn = !radarOn;
			system("cls");
			printf("Trigger: %s\nEsp: %s\nRadar: %s\nSilent Aim: %s\n", triggerOn ? "On" : "Off", espOn ? "On" : "Off", radarOn ? "On" : "Off", silentAimOn ? "On" : "Off");
			PlaySound("..\\click.wav", NULL, SND_FILENAME | SND_SYNC);
		}
		if (GetAsyncKeyState(VK_F9) & 1) {
			silentAimOn = !silentAimOn;
			system("cls");
			printf("Trigger: %s\nEsp: %s\nRadar: %s\nSilent Aim: %s\n", triggerOn ? "On" : "Off", espOn ? "On" : "Off", radarOn ? "On" : "Off", silentAimOn ? "On" : "Off");
			PlaySound("..\\click.wav", NULL, SND_FILENAME | SND_SYNC);
		}

		if (espOn || radarOn) {
			uintptr_t dwGlowManager = RPM<uintptr_t>(moduleBase + dwGlowObjectManager);
			for (int i = 1; i < 32; i++) {
				uintptr_t dwEntity = RPM<uintptr_t>(moduleBase + dwEntityList + i * 0x10);
				int iGlowIndx = RPM<int>(dwEntity + m_iGlowIndex);
				int EnmHealth = RPM<int>(dwEntity + m_iHealth); if (EnmHealth < 1 || EnmHealth > 100) continue;
				int Dormant = RPM<int>(dwEntity + m_bDormant); if (Dormant) continue;
				int EntityTeam = RPM<int>(dwEntity + m_iTeamNum);

				if (LocalTeam != EntityTeam) {
					if (espOn) {
						WPM<glowStructEnemy>(dwGlowManager + (iGlowIndx * 0x38) + 0x4, glowEnm);
					}
					if (radarOn) {
						WPM<bool>(dwEntity + m_bSpotted, true);
					}
				}
			}
		}

		GetAsyncKeyState(VK_MENU);

		if (CrosshairId > 0 && CrosshairId < 32 && LocalTeam != CrosshairTeam && !GetAsyncKeyState(VK_LBUTTON)) {
			if (GetAsyncKeyState(VK_MENU) && triggerOn) {
				mouse_event(MOUSEEVENTF_LEFTDOWN, NULL, NULL, 0, 0);
				mouse_event(MOUSEEVENTF_LEFTUP, NULL, NULL, 0, 0);
				Sleep(175);
			}
		}
	}
}