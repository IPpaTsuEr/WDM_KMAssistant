#pragma once

#include <ntddk.h>
#include <minwindef.h>
#include <ntddkbd.h>
#include <ntddmou.h>
#include <wdmsec.h>

#pragma comment(lib,"wdmsec.lib")

#define _KM_Port_Name L"\\Driver\\i8042prt"

#define _KB_Class_Name L"\\Driver\\kbdclass"
#define _KB_HID_Name L"\\Driver\\kbdhid"

#define _MOU_Class_Name L"\\Driver\\mouclass"
#define _MOU_HID_Name L"\\Driver\\mouhid"

#define _VM_Mouse_Device_Name L"\\Driver\\vmmouse"

#define _Device_Name L"\\Device\\KMControllerDevice"
#define _Device_Symbol_Name L"\\??\\KMControllerLinker"

extern POBJECT_TYPE* IoDriverObjectType;

extern NTSTATUS ObReferenceObjectByName(
	PUNICODE_STRING ObjectName,
	ULONG Attributes,
	PACCESS_STATE AccessState,
	ACCESS_MASK DesiredAccess,
	POBJECT_TYPE ObjectType,
	KPROCESSOR_MODE AccessMode,
	PVOID ParseContext,
	PVOID* Object
);

#define KBCbk(_FunctionName) VOID(* ##_FunctionName)(\
_In_    PDEVICE_OBJECT       DeviceObject,\
_In_    PKEYBOARD_INPUT_DATA InputDataStart,\
_In_    PKEYBOARD_INPUT_DATA InputDataEnd,\
_Inout_ PULONG               InputDataConsumed)

#define MOCbk(_FunctionName) VOID(* ##_FunctionName)(\
_In_    PDEVICE_OBJECT    DeviceObject,\
_In_    PMOUSE_INPUT_DATA InputDataStart,\
_In_    PMOUSE_INPUT_DATA InputDataEnd,\
_Inout_ PULONG            InputDataConsumed)

KBCbk(KeyboardCallback);

MOCbk(MouseCallback);

//VOID(*KeyboardCallback)(
//	_In_    PDEVICE_OBJECT       DeviceObject,
//	_In_    PKEYBOARD_INPUT_DATA InputDataStart,
//	_In_    PKEYBOARD_INPUT_DATA InputDataEnd,
//	_Inout_ PULONG               InputDataConsumed);
//
//VOID(*MouseCallback)(
//	_In_    PDEVICE_OBJECT    DeviceObject,
//	_In_    PMOUSE_INPUT_DATA InputDataStart,
//	_In_    PMOUSE_INPUT_DATA InputDataEnd,
//	_Inout_ PULONG            InputDataConsumed
//	);


typedef struct _KeyInfo {
	USHORT MakeCode;
	USHORT Flags;
}KeyInfo, * PKeyInfo;

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