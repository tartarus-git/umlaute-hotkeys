#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <ShlObj.h>

#include <cstdlib>
#include <cstring>
#include <cstdint>

#include "settings_file_manager.h"

#include "debug_output.h"

#define WINDOW_CLASS_NAME "MESSAGE_ONLY_HOTKEY_WINDOW"

enum class umlaut_id_t : uint8_t {
	A,
	O,
	U,
	a,
	o,
	u,
	SS
};

INPUT input_record_array[] {
	{
		.type = INPUT_KEYBOARD,
		.ki = {
			.wVk = 0,
			.wScan = U'Ä',
			.dwFlags = KEYEVENTF_UNICODE,
			.time = 0,
			.dwExtraInfo = 0
		}
	},
	{
		.type = INPUT_KEYBOARD,
		.ki = {
			.wVk = 0,
			.wScan = U'Ö',
			.dwFlags = KEYEVENTF_UNICODE,
			.time = 0,
			.dwExtraInfo = 0
		}
	},
	{
		.type = INPUT_KEYBOARD,
		.ki = {
			.wVk = 0,
			.wScan = U'Ü',
			.dwFlags = KEYEVENTF_UNICODE,
			.time = 0,
			.dwExtraInfo = 0
		}
	},
	{
		.type = INPUT_KEYBOARD,
		.ki = {
			.wVk = 0,
			.wScan = U'ä',
			.dwFlags = KEYEVENTF_UNICODE,
			.time = 0,
			.dwExtraInfo = 0
		}
	},
	{
		.type = INPUT_KEYBOARD,
		.ki = {
			.wVk = 0,
			.wScan = U'ö',
			.dwFlags = KEYEVENTF_UNICODE,
			.time = 0,
			.dwExtraInfo = 0
		}
	},
	{
		.type = INPUT_KEYBOARD,
		.ki = {
			.wVk = 0,
			.wScan = U'ü',
			.dwFlags = KEYEVENTF_UNICODE,
			.time = 0,
			.dwExtraInfo = 0
		}
	},
	{
		.type = INPUT_KEYBOARD,
		.ki = {
			.wVk = 0,
			.wScan = U'ß',
			.dwFlags = KEYEVENTF_UNICODE,
			.time = 0,
			.dwExtraInfo = 0
		}
	}
};

template <typename element_t, size_t length>
consteval size_t static_array_length(const element_t (&)[length]) { return length; }

bool register_hotkeys(HWND hWnd, const hotkey_settings_t& settings) noexcept {
	hotkey_t data[sizeof(settings) / sizeof(hotkey_t)];
	std::memcpy(data, &settings, sizeof(settings));		// NOTE: VERY IMPORTANT, TO AVOID TYPE PUNNING DISASTER!
	for (uint8_t i = 0; i < static_array_length(data); i++) {
		if (!RegisterHotKey(hWnd, i, data[i].fs_modifiers, data[i].virtual_key_code)) { return false; }
	}
	return true;
}

bool unregister_hotkeys(HWND hWnd) noexcept {
	for (uint8_t i = 0; i < static_array_length(input_record_array); i++) {
		if (!UnregisterHotKey(hWnd, i)) { return false; }
	}
	return true;
}

bool send_umlaut(umlaut_id_t type) noexcept {
	return SendInput(1, input_record_array + (uint8_t)type, sizeof(INPUT));
}

LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept {
	switch (uMsg) {
	case WM_HOTKEY:
		if (wParam >= 0 && wParam < static_array_length(input_record_array)) {
			if (!send_umlaut((umlaut_id_t)wParam)) {
				debuglogger::out << debuglogger::error << "failed to virtually input umlaut" << debuglogger::endl;
				PostQuitMessage(EXIT_FAILURE);
			}
			return 0;
		}
		break;			// NOTE: In case the hotkey ID didn't match what we were expecting, we break and call default handler.
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* lpCmdLine, int nCmdShow) {
	WNDCLASSA windowClass { };
	windowClass.lpfnWndProc = windowProc;
	windowClass.hInstance = hInstance;
	windowClass.lpszClassName = WINDOW_CLASS_NAME;

	if (!RegisterClassA(&windowClass)) {
		debuglogger::out << debuglogger::error << "failed to register window class" << debuglogger::endl;
		return EXIT_FAILURE;
	}

	HWND hWnd = CreateWindowA(windowClass.lpszClassName, nullptr, 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, hInstance, nullptr);
	if (!hWnd) {
		debuglogger::out << debuglogger::error << "couldn't create window" << debuglogger::endl;
		return EXIT_FAILURE;
	}

	char appdata_roaming_folder_path[MAX_PATH];
	// NOTE: I'm aware that this is the old version of the function, but the new version only outputs unicode and there's no pretty way to integrate that with the rest of this, so we're using the old version.
	if (SHGetFolderPathA(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_DEFAULT, appdata_roaming_folder_path) != S_OK) {
		debuglogger::out << debuglogger::error << "couldn't get AppData/Roaming file path" << debuglogger::endl;
	}
	std::string settings_file_path = std::string(appdata_roaming_folder_path) + "\\umlaute_bindings.config";

	hotkey_settings_t hotkey_file_settings = parse_hotkey_settings(settings_file_path.c_str());
	if (hotkey_file_settings == hotkey_settings_t{ }) {
		debuglogger::out << debuglogger::error << "failed to read settings file/parse settings file" << debuglogger::endl;
		return EXIT_FAILURE;
	}

	if (!register_hotkeys(hWnd, hotkey_file_settings)) {
		debuglogger::out << debuglogger::error << "failed to register hotkeys" << debuglogger::endl;
		return EXIT_FAILURE;
	}

	MSG msg = { };
	while (GetMessageA(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}

	if (!unregister_hotkeys(hWnd)) {
		debuglogger::out << debuglogger::error << "failed to unregister hotkeys" << debuglogger::endl;
		return EXIT_FAILURE;
	}
}