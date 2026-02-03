#include "../../include/win64/stacktrace.hpp"

#include <DbgHelp.h>
#include <Windows.h>

#include <cstdint>
#include <vector>

using namespace crashlog::win64;

std::vector<uintptr_t> stacktrace::walkStack(EXCEPTION_POINTERS* ptr)
{
	std::vector<uintptr_t> result;

	CONTEXT context = *ptr->ContextRecord;
	STACKFRAME64 stackframe;
	ZeroMemory(&stackframe, sizeof(stackframe));
	stackframe.AddrPC.Offset = context.Rip;
	stackframe.AddrPC.Mode = AddrModeFlat;
	stackframe.AddrFrame.Offset = context.Rbp;
	stackframe.AddrFrame.Mode = AddrModeFlat;
	stackframe.AddrStack.Offset = context.Rsp;
	stackframe.AddrStack.Mode = AddrModeFlat;

	while (StackWalk64(IMAGE_FILE_MACHINE_AMD64, GetCurrentProcess(), GetCurrentThread(), &stackframe, &context, NULL, NULL, NULL, NULL))
	{
		// TODO: record segment informations?
		result.push_back(stackframe.AddrPC.Offset);
	}

	return result;
}
