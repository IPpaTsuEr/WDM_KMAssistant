#pragma once
#include "Defines.h"
#include "KMHelper.h"

const ULONG IOC_SendKey = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x912, METHOD_BUFFERED, FILE_WRITE_DATA);
const ULONG IOC_SendPoint = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x913, METHOD_BUFFERED, FILE_WRITE_DATA);
const GUID  GUID_CLASS_KM_Assistant = { 0x26e0d1e0L, 0x8189, 0x12e0, {0x89, 0x14, 0x08, 0x00, 0x22, 0x30, 0x19, 0x03} };


extern PDEVICE_OBJECT _Target_KB_Device;
extern PDEVICE_OBJECT _Target_MO_Device;

PDEVICE_OBJECT _Self_Device;

NTSTATUS OnDriverUnload(PDRIVER_OBJECT driverObject) {
	if (_Self_Device != NULL) {
		UNICODE_STRING deviceSmybolName = RTL_CONSTANT_STRING(_Device_Symbol_Name);
		IoDeleteSymbolicLink(&deviceSmybolName);
		IoDeleteDevice(_Self_Device);
		_Self_Device = NULL;
	}
	return STATUS_SUCCESS;
}

NTSTATUS OnDeviceControl(PDEVICE_OBJECT device, PIRP irp) {
	KdPrint(("DeviceControl函数被调用"));
	if (device == _Self_Device) {
		KdPrint(("DeviceControl函数被调用 源自 符号链接设备"));
		PIO_STACK_LOCATION location = IoGetCurrentIrpStackLocation(irp);
		ULONG inputDataConsumed;
		if (location->MajorFunction == IRP_MJ_DEVICE_CONTROL)
		{
			PVOID buffer = irp->AssociatedIrp.SystemBuffer;
			ULONG inSize = location->Parameters.DeviceIoControl.InputBufferLength;
			//ULONG outSize = location->Parameters.DeviceIoControl.OutputBufferLength;

			if (IOC_SendKey == location->Parameters.DeviceIoControl.IoControlCode)
			{
				DbgPrint(buffer);
				PKeyInfo kdata = buffer;
				INT dataCount = inSize / sizeof(KeyInfo);

				PKEYBOARD_INPUT_DATA dataPtr = (PKEYBOARD_INPUT_DATA)ExAllocatePool(NonPagedPool, sizeof(KEYBOARD_INPUT_DATA) * dataCount);
				for (size_t i = 0; i < dataCount; i++)
				{
					dataPtr[i].Flags = kdata[i].Flags;
					dataPtr[i].MakeCode = kdata[i].MakeCode;
				}
				if (_Target_KB_Device != NULL || KeyboardCallback != NULL)
					KeyboardCallback(_Target_KB_Device, dataPtr, dataPtr + dataCount, &inputDataConsumed);
				else {
					KdPrint(("Keyboard Device 或 Callback 为空，操作将被忽略"));
				}
				irp->IoStatus.Information = 0;
				irp->IoStatus.Status = STATUS_SUCCESS;

				IofCompleteRequest(irp, IO_NO_INCREMENT);
				ExFreePoolWithTag(dataPtr, 0);
			}
			else if (IOC_SendPoint == location->Parameters.DeviceIoControl.IoControlCode)
			{
				PPointInfo mdata = buffer;
				INT dataCount = inSize / sizeof(PointInfo);
				PMOUSE_INPUT_DATA dataPtr = (PMOUSE_INPUT_DATA)ExAllocatePool(NonPagedPool, sizeof(MOUSE_INPUT_DATA) * dataCount);
				RtlFillMemory(dataPtr, sizeof(MOUSE_INPUT_DATA) * dataCount, 0);

				for (size_t i = 0; i < dataCount; i++)
				{
					dataPtr[i].Flags = mdata[i].Flags;
					dataPtr[i].Buttons = mdata[i].Buttons;
					dataPtr[i].LastX = mdata[i].LastX;
					dataPtr[i].LastY = mdata[i].LastY;

				}
				if (_Target_MO_Device != NULL || MouseCallback != NULL)
					MouseCallback(_Target_MO_Device, dataPtr, dataPtr + dataCount, &inputDataConsumed);
				else {
					KdPrint(("Mouse Device 或 Callback 为空，操作将被忽略"));
				}
				irp->IoStatus.Information = 0;
				irp->IoStatus.Status = STATUS_SUCCESS;

				IofCompleteRequest(irp, IO_NO_INCREMENT);
				ExFreePoolWithTag(dataPtr, 0);
			}
			else
			{
				return STATUS_NOT_SUPPORTED;
			}
		}
	}
	return STATUS_SUCCESS;
}

NTSTATUS OnSymbolDeviceCreate(PDEVICE_OBJECT device, PIRP irp) {
	KdPrint(("SymbolDeviceCreated函数被调用"));
	return STATUS_SUCCESS;
}

NTSTATUS OnSymbolDeviceClose(PDEVICE_OBJECT device, PIRP irp) {
	KdPrint(("SymbolDeviceClosed函数被调用"));
	return STATUS_SUCCESS;
}

NTSTATUS DefaultFunction(PDEVICE_OBJECT device, PIRP irp) {
	KdPrint(("DefaultFunction函数被调用"));
	return STATUS_SUCCESS;
}

NTSTATUS CreateDevice(PDRIVER_OBJECT driver) {
	UNICODE_STRING deviceName = RTL_CONSTANT_STRING(_Device_Name);
	//UNICODE_STRING sddl = RTL_CONSTANT_STRING(L"D:P(A;;GA;;;SY)(A;;GRGWGX;;;BA)(A;;GR;;;WD)");//高权限用户才能打开
	UNICODE_STRING sddl = RTL_CONSTANT_STRING(L"D:P(A;;GA;;;WD)");//任何用户都能打开
	NTSTATUS status = IoCreateDeviceSecure(
		driver,
		0,
		&deviceName,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		FALSE,
		&sddl,
		(LPCGUID)&GUID_CLASS_KM_Assistant,
		&_Self_Device
	);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("创建设备失败 %x", status));
		return status;
	}
	KdPrint(("创建设备成功"));
	UNICODE_STRING deviceSmybolName = RTL_CONSTANT_STRING(_Device_Symbol_Name);

	status = IoCreateSymbolicLink(
		&deviceSmybolName,
		&deviceName);

	if (!NT_SUCCESS(status)) {
		KdPrint(("创建符号链接失败 %x", status));
		IoDeleteDevice(_Self_Device);
		_Self_Device = NULL;
		return status;
	}
	KdPrint(("创建符号链接成功"));
	return status;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driverObject, PUNICODE_STRING registry) {

	PAGED_CODE();
	driverObject->DriverUnload = OnDriverUnload;

	for (size_t i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		driverObject->MajorFunction[i] = DefaultFunction;
	}

	driverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = OnDeviceControl;
	driverObject->MajorFunction[IRP_MJ_CREATE] = OnSymbolDeviceCreate;
	driverObject->MajorFunction[IRP_MJ_CLOSE] = OnSymbolDeviceClose;

	if (!NT_SUCCESS(CreateDevice(driverObject)))
	{
		return STATUS_FAILED_DRIVER_ENTRY;
	}
	//KdBreakPoint();
	InitCallbackPoint();

	return STATUS_SUCCESS;
}