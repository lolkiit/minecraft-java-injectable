#pragma once

#ifndef CRASHLOG_NO_WINDOWS_H
#include <Windows.h>
#endif

#include "../exception.hpp"

namespace crashlog::win64
{
	void initialize();

	crashlog::Exception parse(EXCEPTION_POINTERS* ptr);
};
