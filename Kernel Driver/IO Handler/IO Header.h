#pragma once

#include "../Core/Core.h"

namespace DH
{
	namespace CmdData
	{
		struct IOCOMMAND_t
		{
			ULONG NameHash;
			ULONG ProcessID;
			ULONG CommandID;
			PVOID Src, Dst;
			ULONG Size;
			ULONG Ret;
		};

		struct ModInfo_t
		{
			ULONG64 ModBase;
			ULONG ModSize;
		};
	}

	NTSTATUS __cdecl hookHandler(CmdData::IOCOMMAND_t* pBuff);

}