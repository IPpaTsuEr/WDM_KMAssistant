
#include <iostream>
#include<Windows.h>
#include<thread>

#include "Defines.h"


int main()
{
	int v = 0;
	std::cout << "Hello World!\n Press Any Key To Conect Device";
	std::cin >> v;
	auto handle = CreateFile(
		L"\\\\.\\KMControllerLinker",
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (handle == INVALID_HANDLE_VALUE) {
		std::cout << "Open Device Fialed\n";

		std::cin >> v;
		return 0;
	}
	KeyData keys[] = {
		{0x02,KEY_MAKE },{ 0x02,KEY_BREAK},
		{0x03,KEY_MAKE },{ 0x03,KEY_BREAK},
		{0x04,KEY_MAKE },{ 0x04,KEY_BREAK},
		{0x05,KEY_MAKE },{ 0x05,KEY_BREAK},
		{0x06,KEY_MAKE },{ 0x06,KEY_BREAK},
	};
	PointInfo mous[] = {
		{ MOUSE_MOVE_RELATIVE,
		0,
		100,300
		},
		{ MOUSE_MOVE_ABSOLUTE ,
		0,
		(LONG)(800 * 0xffff / GetSystemMetrics(SM_CXSCREEN)),(LONG)(600 * 0xffff / GetSystemMetrics(SM_CYSCREEN))
		},

		//{MOUSE_ATTRIBUTES_CHANGED,{.ButtonFlags = MOUSE_RIGHT_BUTTON_DOWN,.ButtonData = 0},0,0},
		//{MOUSE_ATTRIBUTES_CHANGED,{.ButtonFlags = MOUSE_RIGHT_BUTTON_UP,.ButtonData = 0},0,0},

		//{MOUSE_ATTRIBUTES_CHANGED,{.ButtonFlags = MOUSE_LEFT_BUTTON_DOWN,.ButtonData = 0},0,0},
		//{MOUSE_ATTRIBUTES_CHANGED,{.ButtonFlags = MOUSE_LEFT_BUTTON_UP,.ButtonData = 0},0,0},

		//{MOUSE_ATTRIBUTES_CHANGED,{.ButtonFlags = MOUSE_WHEEL,.ButtonData = 127},-1,-1},
	};
	DWORD renturn_len = 0;

	auto IOC_SendKey = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x912, METHOD_BUFFERED, FILE_WRITE_DATA);
	auto IOC_SendPoint = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x913, METHOD_BUFFERED, FILE_WRITE_DATA);

	std::cout << "Press Any Key To Send Key Data\n";
	std::cin >> v;
	auto result = DeviceIoControl(handle, IOC_SendKey, (PVOID)keys, sizeof(keys), NULL, 0, &renturn_len, 0);
	if (result) {
		std::cout << "Send Success\n";
	}
	else {
		std::cout << "Send Failed\n";
	}

	std::cout << "Press Any Key To Send Mouse Data\n";
	std::cin >> v;
	for (size_t i = 0; i < (sizeof(mous) / sizeof(PointInfo)); i++)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
		result = DeviceIoControl(handle, IOC_SendPoint, (PVOID)&mous[i], sizeof(PointInfo), NULL, 0, &renturn_len, 0);
		if (result) {
			std::cout << "Send Success " << i << "\n";
		}
		else {
			std::cout << "Send Failed" << i << "\n";
		}

	}

	if (CloseHandle(handle)) {
		std::cout << "Device Closed\n";
	}


	std::cin >> v;
	return 0;
}