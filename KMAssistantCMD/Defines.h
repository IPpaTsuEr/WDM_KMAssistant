#pragma once

#include<Windows.h>

#define KEY_MAKE 0
#define KEY_BREAK 1

#define MOUSE_MOVE_RELATIVE 0
#define MOUSE_MOVE_ABSOLUTE 1
#define MOUSE_ATTRIBUTES_CHANGED 0x04

#define MOUSE_LEFT_BUTTON_DOWN 0x0001
#define MOUSE_LEFT_BUTTON_UP 0x0002

#define MOUSE_RIGHT_BUTTON_DOWN 0x0004
#define MOUSE_RIGHT_BUTTON_UP 0x0008

#define MOUSE_MIDDLE_BUTTON_DOWN 0x0010
#define MOUSE_MIDDLE_BUTTON_UP 0x0020

#define MOUSE_WHEEL 0x0400
#define MOUSE_HWHEEL 0x0800


typedef struct _KeyData {
	USHORT MakeCode;
	USHORT Flags;
}KeyData, * PKeyData;

typedef struct _PointInfo {
	USHORT Flags;
	union {
		ULONG Buttons;
		struct {
			USHORT ButtonFlags;
			USHORT ButtonData;
		};
	};

	LONG   LastX;
	LONG   LastY;
}PointInfo, * PPointInfo;