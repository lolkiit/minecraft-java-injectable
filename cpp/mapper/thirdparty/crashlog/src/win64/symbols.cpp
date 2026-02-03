#include "../../include/win64/symbols.hpp"

#include <Windows.h>
#include <DbgHelp.h>
#include <psapi.h>

#include <optional>

#include "../../include/address.hpp"

using namespace crashlog::win64;

void symbols::loadAllSymbols()
{
	HMODULE modules[1024];
	DWORD capacityBytesNeeded;
	if (EnumProcessModules(GetCurrentProcess(), modules, sizeof(modules), &capacityBytesNeeded))
	{
		DWORD numModules = capacityBytesNeeded / sizeof(HMODULE);
		for (DWORD i = 0; i < numModules; i++) 
		{
			TCHAR moduleFileName[MAX_PATH];
			if (!GetModuleFileNameEx(GetCurrentProcess(), modules[i], moduleFileName, sizeof(moduleFileName) / sizeof(TCHAR)))
			{
				continue;
			}

			DWORD64 baseAddr = (DWORD64)modules[i];
			SymLoadModuleEx(GetCurrentProcess(), NULL, moduleFileName, NULL, baseAddr, 0, NULL, 0);

			IMAGEHLP_MODULE64 modInfo;
			memset(&modInfo, 0, sizeof(modInfo));
			modInfo.SizeOfStruct = sizeof(modInfo);
			SymGetModuleInfo64(GetCurrentProcess(), baseAddr, &modInfo);
		}
	}
}

std::pair<std::string, uint64_t> symbols::moduleInfo(uintptr_t address) 
{
    HANDLE process = GetCurrentProcess();
    IMAGEHLP_MODULE64 moduleInfo = { 0 };
    moduleInfo.SizeOfStruct = sizeof(moduleInfo);
    if (!SymGetModuleInfo64(process, address, &moduleInfo)) 
	{
        return { "", 0x0 };
    }
    uint64_t offset = address - moduleInfo.BaseOfImage;
    return { std::string(moduleInfo.ModuleName), offset };
}

std::optional<std::string> symbols::symbol(uintptr_t address) 
{
	BYTE symbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME] = {};
	SYMBOL_INFO* symbolInfo = (SYMBOL_INFO*)symbolBuffer;
	symbolInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
	symbolInfo->MaxNameLen = MAX_SYM_NAME;

	bool symFromAddrResult = SymFromAddr(GetCurrentProcess(), address, nullptr, symbolInfo);
	return symFromAddrResult ? std::optional<std::string>(symbolInfo->Name) : std::nullopt;
}

std::optional<crashlog::LineInfo> symbols::line(uintptr_t address)
{
	IMAGEHLP_LINE64 lineInfo;
	memset(&lineInfo, 0, sizeof(IMAGEHLP_LINE64));
	lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
	DWORD dwDisplacement;
	bool getLineFromAddr64Result = SymGetLineFromAddr64(GetCurrentProcess(), address, &dwDisplacement, &lineInfo);
	return getLineFromAddr64Result ? std::optional<crashlog::LineInfo>({lineInfo.FileName, static_cast<uint32_t>(lineInfo.LineNumber)}) : std::nullopt;
}
