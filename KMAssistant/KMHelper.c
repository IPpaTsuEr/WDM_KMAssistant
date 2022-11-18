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
		KdPrint(("û���ҵ� %wZ,%x", targetDeviceName, status));

		return FALSE;
	}
	ObfReferenceObject(*driver);
	return TRUE;
}

VOID SeachCallbackPoint(PUNICODE_STRING	targetHIDOrPortName, PUNICODE_STRING targetClassName, BOOLEAN isKbPoint) {
	KdPrint(("�����豸 %wZ <> %wZ", targetHIDOrPortName, targetClassName));
	//������ڵ�Port��HID�¶���һ��
	UNICODE_STRING vmMouse;
	RtlInitUnicodeString(&vmMouse, _VM_Mouse_Device_Name);

	PDRIVER_OBJECT findedDriver;
	//����kbdHID �� Port ����
	if (FindDevice(targetHIDOrPortName, &findedDriver)) {
		PDRIVER_OBJECT kbDriver;
		//����kbdclass����
		if (FindDevice(targetClassName, &kbDriver)) {
			//���һ����ӵ��豸
			PDEVICE_OBJECT targetDevice = kbDriver->DeviceObject;
			PDEVICE_OBJECT TargetHostDevicePoint = NULL;
			BOOLEAN isVMDevice = FALSE;
			do {
				BOOL isFinded = FALSE;

				//�ײ��������豸ջ
				PDEVICE_OBJECT HIDDevicePoint = findedDriver->DeviceObject;

				while (HIDDevicePoint != NULL)
				{
					if (!isKbPoint)
					{
						LONG cValue = RtlCompareUnicodeString(
							&HIDDevicePoint->AttachedDevice->DriverObject->DriverName,
							&vmMouse,
							TRUE);

						KdPrint(("�Ƚ��豸 %wZ ,���Ϊ %s\n", HIDDevicePoint->AttachedDevice->DriverObject->DriverName, cValue == 0 ? "True" : "False"));

						if (cValue == 0)
						{
							//�ҵ��˱����ӵ��豸
							if (HIDDevicePoint->AttachedDevice->AttachedDevice == targetDevice)
							{
								KdPrint(("�ҵ��˱����ӵ�VM�豸 %p\n", targetDevice));
								isFinded = TRUE;
								isVMDevice = TRUE;
								break;
							}
							else {
								KdPrint(("�ҵ��ı����ӵ�VM�豸 %p ��ƥ��\n", targetDevice));
							}
						}
					}
					//�ҵ��˱����ӵ��豸
					if (HIDDevicePoint->AttachedDevice == targetDevice)
					{
						KdPrint(("�ҵ��˱����ӵ��豸 %p\n", targetDevice));
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
					//ע��64��32λ��ָ��ĳ���
#ifdef _WIN64

					PVOID nextVptr = *(PVOID*)(ptr + 8);
#else

					PVOID nextVptr = *(PVOID*)(ptr + 4);
#endif // _Win64

					if (vptr == targetDevice &&
						(nextVptr >= kbDriver->DriverStart) &&
						(nextVptr <=(PVOID) ((PUCHAR)kbDriver->DriverStart + kbDriver->DriverSize))) {
						//�ҵ��ص���ַ
						if (isKbPoint)
						{
							//KeyboardCallback = (KBCbk()) nextVptr;
							_Target_KB_Device = targetDevice;
							KdPrint(("�����ü��̻ص���ַ %p\n", KeyboardCallback));
							break;
						}
						else
						{
							//MouseCallback = (MOCbk( ))nextVptr;
							_Target_MO_Device = targetDevice;
							//if (isVMDevice) {
							//	targetDevice = TargetHostDevicePoint->AttachedDevice;
							//}
							KdPrint(("���������ص���ַ %p\n", MouseCallback));
							break;
						}
					}
				}
				KdPrint(("�ص���ַ���ҽ���\n"));
			}
			else
			{
				KdPrint(("û���ҵ�ƥ��ĸ����豸\n"));
			}
		}
	}
}


VOID InitCallbackPoint() {

	UNICODE_STRING targetPortName = RTL_CONSTANT_STRING(_KM_Port_Name);
	UNICODE_STRING	targetKBName = RTL_CONSTANT_STRING(_KB_Class_Name);
	UNICODE_STRING	targetMOName = RTL_CONSTANT_STRING(_MOU_Class_Name);
	PDRIVER_OBJECT findedDriver;
	//����ps2
	if (FindDevice(&targetPortName, &findedDriver)) {
		//����kbdclass����
		SeachCallbackPoint(&targetPortName, &targetKBName, TRUE);
		//����mouclass����
		SeachCallbackPoint(&targetPortName, &targetMOName, FALSE);
	}
	//����hid
	else
	{
		UNICODE_STRING	targetKBHIDName = RTL_CONSTANT_STRING(_KB_HID_Name);
		//����kbdclass����
		SeachCallbackPoint(&targetKBHIDName, &targetKBName, TRUE);

		UNICODE_STRING	targetMOHIDName = RTL_CONSTANT_STRING(_MOU_HID_Name);
		//����mouclass����
		SeachCallbackPoint(&targetMOHIDName, &targetMOName, FALSE);

	}
}
