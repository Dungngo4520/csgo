#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include "../Offsets.h"

#pragma comment(lib,"winmm.lib")

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

int main() {
	hwnd = FindWindowA(NULL, "Counter-Strike: Global Offensive");
	GetWindowThreadProcessId(hwnd, &procId);
	moduleBase = GetModuleBaseAddress("client.dll");
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);
	BOOL ToggleOn = FALSE;
	printf("Toggle: %s\n", ToggleOn ? "On" : "Off");

	while (!GetAsyncKeyState(VK_END)) {
		int CrosshairId = getCrosshairID(getLocalPlayer());
		int CrosshairTeam = getTeam(getPlayer(CrosshairId - 1));
		int LocalTeam = getTeam(getLocalPlayer());

		if (GetAsyncKeyState(VK_F10) & 1) {
			ToggleOn = !ToggleOn;
			system("cls");
			printf("Toggle: %s\n", ToggleOn ? "On" : "Off");
			PlaySound("..\\click.wav", NULL, SND_FILENAME | SND_SYNC);
		}

		if (CrosshairId > 0 && CrosshairId < 32 && LocalTeam != CrosshairTeam && !GetAsyncKeyState(VK_LBUTTON)) {
			if (GetAsyncKeyState(VK_MENU) || ToggleOn) {
				mouse_event(MOUSEEVENTF_LEFTDOWN, NULL, NULL, 0, 0);
				mouse_event(MOUSEEVENTF_LEFTUP, NULL, NULL, 0, 0);
				Sleep(150);
			}
		}
	}
}