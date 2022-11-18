#include "KMHelper.h"

PDEVICE_OBJECT _Target_KB_Device;
PDEVICE_OBJECT _Target_MO_Device;

BOOLEAN FindDevice(PUNICODE_STRING targetDeviceName, OUT PDRIVER_OBJECT* driver) {

	NTSTATUS status = ObReferenceObjectByName(
		targetDeviceName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		0,
		*IoDriverObjectType,
		KernelMode,
		NULL,
		(PVOID*)driver
	);
	if (!NT_SUCCESS(status) || *driver == NULL) {
		KdPrint(("没有找到 %wZ,%x", targetDeviceName, status));

		return FALSE;
	}
	ObfReferenceObject(*driver);
	return TRUE;
}

VOID SeachCallbackPoint(PUNICODE_STRING	targetHIDOrPortName, PUNICODE_STRING targetClassName, BOOLEAN isKbPoint) {
	KdPrint(("查找设备 %wZ <> %wZ", targetHIDOrPortName, targetClassName));
	//虚拟机内的Port或HID下多了一层
	UNICODE_STRING vmMouse;
	RtlInitUnicodeString(&vmMouse, _VM_Mouse_Device_Name);

	PDRIVER_OBJECT findedDriver;
	//查找kbdHID 或 Port 驱动
	if (FindDevice(targetHIDOrPortName, &findedDriver)) {
		PDRIVER_OBJECT kbDriver;
		//查找kbdclass驱动
		if (FindDevice(targetClassName, &kbDriver)) {
			//最后一个添加的设备
			PDEVICE_OBJECT targetDevice = kbDriver->DeviceObject;
			PDEVICE_OBJECT TargetHostDevicePoint = NULL;
			BOOLEAN isVMDevice = FALSE;
			do {
				BOOL isFinded = FALSE;

				//底层驱动的设备栈
				PDEVICE_OBJECT HIDDevicePoint = findedDriver->DeviceObject;

				while (HIDDevicePoint != NULL)
				{
					if (!isKbPoint)
					{
						LONG cValue = RtlCompareUnicodeString(
							&HIDDevicePoint->AttachedDevice->DriverObject->DriverName,
							&vmMouse,
							TRUE);

						KdPrint(("比较设备 %wZ ,结果为 %s\n", HIDDevicePoint->AttachedDevice->DriverObject->DriverName, cValue == 0 ? "True" : "False"));

						if (cValue == 0)
						{
							//找到了被附加的设备
							if (HIDDevicePoint->AttachedDevice->AttachedDevice == targetDevice)
							{
								KdPrint(("找到了被附加的VM设备 %p\n", targetDevice));
								isFinded = TRUE;
								isVMDevice = TRUE;
								break;
							}
							else {
								KdPrint(("找到的被附加的VM设备 %p 不匹配\n", targetDevice));
							}
						}
					}
					//找到了被附加的设备
					if (HIDDevicePoint->AttachedDevice == targetDevice)
					{
						KdPrint(("找到了被附加的设备 %p\n", targetDevice));
						isFinded = TRUE;
						break;
					}
					HIDDevicePoint = HIDDevicePoint->NextDevice;
				}
				if (isFinded)
				{
					TargetHostDevicePoint = HIDDevicePoint;
					break;
				}
				targetDevice = targetDevice->NextDevice;

			} while (targetDevice != NULL);


			if (TargetHostDevicePoint)
			{
				UCHAR* ptr = (UCHAR*)(TargetHostDevicePoint->DeviceExtension);
				for (size_t i = 0; i < kbDriver->DriverSize; ptr++)
				{
					PVOID vptr = *(PVOID*)ptr;
					//注意64与32位下指针的长度
#ifdef _WIN64

					PVOID nextVptr = *(PVOID*)(ptr + 8);
#else

					PVOID nextVptr = *(PVOID*)(ptr + 4);
#endif // _Win64

					if (vptr == targetDevice &&
						(nextVptr >= kbDriver->DriverStart) &&
						(nextVptr <=(PVOID) ((PUCHAR)kbDriver->DriverStart + kbDriver->DriverSize))) {
						//找到回调地址
						if (isKbPoint)
						{
							//KeyboardCallback = (KBCbk()) nextVptr;
							_Target_KB_Device = targetDevice;
							KdPrint(("已设置键盘回调地址 %p\n", KeyboardCallback));
							break;
						}
						else
						{
							//MouseCallback = (MOCbk( ))nextVptr;
							_Target_MO_Device = targetDevice;
							//if (isVMDevice) {
							//	targetDevice = TargetHostDevicePoint->AttachedDevice;
							//}
							KdPrint(("已设置鼠标回调地址 %p\n", MouseCallback));
							break;
						}
					}
				}
				KdPrint(("回调地址查找结束\n"));
			}
			else
			{
				KdPrint(("没有找到匹配的附加设备\n"));
			}
		}
	}
}


VOID InitCallbackPoint() {

	UNICODE_STRING targetPortName = RTL_CONSTANT_STRING(_KM_Port_Name);
	UNICODE_STRING	targetKBName = RTL_CONSTANT_STRING(_KB_Class_Name);
	UNICODE_STRING	targetMOName = RTL_CONSTANT_STRING(_MOU_Class_Name);
	PDRIVER_OBJECT findedDriver;
	//查找ps2
	if (FindDevice(&targetPortName, &findedDriver)) {
		//查找kbdclass驱动
		SeachCallbackPoint(&targetPortName, &targetKBName, TRUE);
		//查找mouclass驱动
		SeachCallbackPoint(&targetPortName, &targetMOName, FALSE);
	}
	//查找hid
	else
	{
		UNICODE_STRING	targetKBHIDName = RTL_CONSTANT_STRING(_KB_HID_Name);
		//查找kbdclass驱动
		SeachCallbackPoint(&targetKBHIDName, &targetKBName, TRUE);

		UNICODE_STRING	targetMOHIDName = RTL_CONSTANT_STRING(_MOU_HID_Name);
		//查找mouclass驱动
		SeachCallbackPoint(&targetMOHIDName, &targetMOName, FALSE);

	}
}
