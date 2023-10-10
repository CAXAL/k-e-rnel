#include "Common.h"

NTSTATUS NTAPI DriverEntry(PVOID pParam1, PVOID pParam2)
{
	DH::Core::Initialize((DWORD64)pParam1);

	PFILE_OBJECT pFileObj = NULL;
	PDEVICE_OBJECT pObject = NULL;
	UNICODE_STRING DeviceString = RTL_CONSTANT_STRING(L"\\Device\\Null");
	DH::Fn::IoGetDeviceObjectPointerFn(&DeviceString, FILE_READ_DATA, &pFileObj, &pObject);

	DH::Fn::ObfDereferenceObjectFn(pFileObj);

	SYSTEM_FIRMWARE_TABLE_HANDLER TableHandler = {};
	TableHandler.DriverObject = pObject->DriverObject;
	TableHandler.FirmwareTableHandler = (PFNFTH)DH::hookHandler;
	TableHandler.ProviderSignature = 'G?R4';
	TableHandler.Register = true;

	DH::Fn::ZwSetSystemInformationFn(SYSTEM_INFORMATION_CLASS::SystemRegisterFirmwareTableInformationHandler, &TableHandler, sizeof(TableHandler));


	return 0;
}