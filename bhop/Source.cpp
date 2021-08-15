#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include "../Offsets.h"
#include <math.h>

#define VK_A 0x41
#define VK_D 0x44

uintptr_t moduleBase;
DWORD procId;
HWND hwnd;
HANDLE hProcess;


uintptr_t GetModuleBaseAddress(const char* modName) {
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
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

template<typename T> T RPM(SIZE_T address) {
	T buffer;
	ReadProcessMemory(hProcess, (LPCVOID)address, &buffer, sizeof(T), NULL);
	return buffer;
}

template<typename T> void WPM(SIZE_T address, T buffer) {
	WriteProcessMemory(hProcess, (LPVOID)address, &buffer, sizeof(buffer), NULL);
}

int getFlags() {
	return RPM<int>(moduleBase + dwEntityList + m_fFlags);
}

int main() {
	hwnd = FindWindowA(NULL, "Counter-Strike: Global Offensive");
	GetWindowThreadProcessId(hwnd, &procId);
	moduleBase = GetModuleBaseAddress("client.dll");
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);
	uintptr_t localPlayer = RPM<uintptr_t>(moduleBase + dwEntityList);
	uintptr_t buffer;
	RECT rect;
	POINT center, cursor;

	if (GetWindowRect(hwnd, &rect)) {
		center.x = (rect.right - rect.left) / 2 + rect.left;
		center.y = (rect.bottom - rect.top) / 2 + rect.top;
	}
	else {
		center.x = 960;
		center.y = 540;
	}
	int strafe = 0; // 0 mean not strafe, 1 strafe left, 2 strafe right

	while (!GetAsyncKeyState(VK_END)) {
		int flags = RPM<int>(localPlayer + m_fFlags);

		if (flags & 1) {
			buffer = 5;
		}
		else {
			buffer = 4;
		}
		if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
			WPM(moduleBase + dwForceJump, buffer);
		}


		if (flags == 256) {
			GetCursorPos(&cursor);
			if (abs(cursor.x - center.x) > 10) {
				if (center.x > cursor.x) {
					strafe = 1;
				}
				if (center.x < cursor.x) {
					strafe = 2;
				}
				printf("%d\n", center.x - cursor.x);
				if (strafe == 1) {
					keybd_event(VK_A, MapVirtualKey(VK_A, MAPVK_VK_TO_VSC), KEYEVENTF_EXTENDEDKEY, 0);
					Sleep(1);
					keybd_event(VK_A, MapVirtualKey(VK_A, MAPVK_VK_TO_VSC), KEYEVENTF_KEYUP, 0);
				}
				if (strafe == 2) {
					keybd_event(VK_D, MapVirtualKey(VK_D, MAPVK_VK_TO_VSC), KEYEVENTF_EXTENDEDKEY, 0);
					Sleep(1);
					keybd_event(VK_D, MapVirtualKey(VK_D, MAPVK_VK_TO_VSC), KEYEVENTF_KEYUP, 0);
				}
			}
		}

		if (flags != 256) {
			strafe = 0;
		}
	}
}