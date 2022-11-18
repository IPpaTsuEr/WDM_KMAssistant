#pragma once
#include "Defines.h"

BOOLEAN FindDevice(PUNICODE_STRING targetDeviceName, OUT PDRIVER_OBJECT* driver);

VOID SeachCallbackPoint(PUNICODE_STRING	targetHIDOrPortName, PUNICODE_STRING targetClassName, BOOLEAN isKbPoint);

VOID InitCallbackPoint();
