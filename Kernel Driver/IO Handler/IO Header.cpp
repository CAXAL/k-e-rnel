#include "IO Header.h"
namespace DH
{
	CmdData::ModInfo_t BBGetUserModule(PEPROCESS pProcess, const char* ModName)
	{

		if (IoIs32bitProcess(nullptr))
		{

			PPEB32 pPeb32 = (PPEB32)Fn::PsGetProcessWow64ProcessFn(pProcess);
			if (!pPeb32 || !pPeb32->Ldr)
				goto FastExit;

			for (PLIST_ENTRY32 pListEntry = (PLIST_ENTRY32)((PPEB_LDR_DATA32)pPeb32->Ldr)->InLoadOrderModuleList.Flink; pListEntry != &((PPEB_LDR_DATA32)pPeb32->Ldr)->InLoadOrderModuleList; pListEntry = (PLIST_ENTRY32)pListEntry->Flink)
			{
				PLDR_DATA_TABLE_ENTRY32 pEntry = CONTAINING_RECORD(pListEntry, LDR_DATA_TABLE_ENTRY32, InLoadOrderLinks);

				if (Utils::R::strcmp_r(ModName, (PWCH)pEntry->BaseDllName.Buffer, false))
					return { pEntry->DllBase, pEntry->SizeOfImage };
			}
		}
		else
		{

			PPEB pPeb = Fn::PsGetProcessPebFn(pProcess);
			if (!pPeb || !pPeb->Ldr)
				goto FastExit;

			KAPC_STATE state;
			for (PLIST_ENTRY pListEntry = pPeb->Ldr->InLoadOrderModuleList.Flink; pListEntry != &pPeb->Ldr->InLoadOrderModuleList; pListEntry = pListEntry->Flink)
			{
				PLDR_DATA_TABLE_ENTRY pEntry = CONTAINING_RECORD(pListEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

				if (Utils::R::strcmp_r(ModName, pEntry->BaseDllName.Buffer, false))
					return { (ULONG64)pEntry->DllBase, pEntry->SizeOfImage };
				
			}
		}

		//failed
	FastExit:
		return CmdData::ModInfo_t{ 0, 0 };
	}

	enum IO_CMD : ULONG
	{
		CMD_NONE = 0x100000,
		CMD_READ_VM = 0x200000,
		CMD_WRITE_VM = 0x300000,
		CMD_GET_MOD_INFO = 0x600000,
		CMD_CHECKSTATUS = 0x900000,
	};


	NTSTATUS __cdecl hookHandler(CmdData::IOCOMMAND_t* Buff)
	{
		PEPROCESS Process = nullptr, CurProcess = IoGetCurrentProcess();

		if (!Fn::PsLookupProcessByProcessIdFn((HANDLE)Buff->ProcessID, &Process) && Process && !PsAcquireProcessExitSynchronization(Process))
		{
			size_t CopySize;

			switch (Buff->CommandID)
			{
			case CMD_READ_VM:
			{
				Fn::MmCopyVirtualMemoryFn(Process, Buff->Src, CurProcess, Buff->Dst, Buff->Size, MODE::UserMode, &CopySize);
			} break;

			case CMD_WRITE_VM:
			{
				Fn::MmCopyVirtualMemoryFn(CurProcess, Buff->Src, Process, Buff->Dst, Buff->Size, MODE::UserMode, &CopySize);
			} break;

			case CMD_GET_MOD_INFO:
			{

				char ModName[512];
				CmdData::ModInfo_t ModData = { 0, 0 };
				SIZE_T StrSize = strlen((const char*)Buff->Src) + 1;

				if (StrSize <= 512)
				{

					__movsb((PUCHAR)ModName, (PUCHAR)Buff->Src, StrSize);
					
					KAPC_STATE state;
					KeAttachProcess(Process);
					ModData = BBGetUserModule(Process, ModName);
					KeDetachProcess();
				}

				*(CmdData::ModInfo_t*)Buff->Dst = ModData;
			} break;

			//check loaded
			case CMD_CHECKSTATUS:
			{
				*(DWORD64*)Buff->Dst = (DWORD64)((DWORD64)Buff->Src * (DWORD64)Buff->Src);
			} break;
			}

			//cleanup
			Fn::ObfDereferenceObjectFn(Process);
			Fn::PsReleaseProcessExitSynchronizationFn(Process);
		}
		return STATUS_ACCESS_DENIED;
	}
}