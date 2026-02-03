#pragma once

#include <cstdint>
#include <vector>

#ifndef CRASHLOG_NO_WINDOWS_H
#include <Windows.h>
#endif

namespace crashlog::win64::stacktrace 
{
	std::vector<uintptr_t> walkStack(EXCEPTION_POINTERS* ptr);
};
