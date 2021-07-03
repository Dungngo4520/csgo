#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include "../Offsets.h"

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

int main() {
	hwnd = FindWindowA(NULL, "Counter-Strike: Global Offensive");
	GetWindowThreadProcessId(hwnd, &procId);
	moduleBase = GetModuleBaseAddress("client.dll");
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);
	uintptr_t buffer;
	RECT rect;
	POINT center;
	POINT cursor;
	INPUT moveLeft, moveRight;
	moveLeft.type = INPUT_KEYBOARD;
	moveLeft.ki.wVk = 0x41;
	moveLeft.ki.dwFlags = KEYEVENTF_SCANCODE;
	moveRight.type = INPUT_KEYBOARD;
	moveRight.ki.wVk = 0x44;
	moveRight.ki.dwFlags = KEYEVENTF_SCANCODE;

	if (GetWindowRect(hwnd, &rect)) {
		center.x = (rect.right - rect.left) / 2 + rect.left;
		center.y = (rect.bottom - rect.top) / 2 + rect.top;
	}
	else {
		center.x = 960;
		center.y = 540;
	}

	while (!GetAsyncKeyState(VK_END)) {
		uintptr_t localPlayer = RPM<uintptr_t>(moduleBase + dwEntityList);
		int flags = RPM<int>(localPlayer + m_fFlags);

		if (flags & 1) {
			buffer = 5;
		} else {
			buffer = 4;
		}

		if (flags == 256) {
			GetCursorPos(&cursor);
			if (cursor.x < center.x) {
				keybd_event(0x44, 0, KEYEVENTF_KEYUP, 0);
				keybd_event(0x41, 0, 0, 0);
			}
			if (cursor.x > center.x) {
				keybd_event(0x41, 0, KEYEVENTF_KEYUP, 0);
				keybd_event(0x44, 0, 0, 0);
			}
		}

		if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
			WPM(moduleBase + dwForceJump, buffer);
		}
	}
}