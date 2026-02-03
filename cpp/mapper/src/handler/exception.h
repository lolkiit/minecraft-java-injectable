/*
	* ==================================== *
		* Project: Nexus Client (Mapper)
		* File: mapper/src/handler/exception.h
		* Purpose: Unhandled exception filter. Writes crash log (stack trace, registers) and shows message.
		* Author: lolkit
	* ==================================== *
*/

#pragma once
#include <Windows.h>
#include <sstream>
#include <errhandlingapi.h>
#include <fstream>
#include <iomanip>

#include <crashlog/include/crashlog.hpp>

class exception_handler {
private:
	static LONG WINAPI exception_filter(EXCEPTION_POINTERS* exceptionInfo)
	{
		auto exception = crashlog::parse(exceptionInfo);

		SYSTEMTIME st;
		GetLocalTime(&st);

		char filename[256];
		sprintf_s(filename, ("crash_%04d%02d%02d_%02d%02d%02d.log"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

		std::ofstream file(filename);
		if (file.is_open())
		{
			file << ("=== CRASH LOG ===") << std::endl;
			file << ("Exception Code: 0x") << std::hex << exception.exceptionMetadata.exceptionCode << std::dec << std::endl;
			file << ("Exception Name: ") << exception.exceptionMetadata.exceptionName << std::endl;
			file << ("Address: 0x") << std::hex << exception.exceptionMetadata.address.address << std::dec << std::endl;

			file << ("\n=== STACK TRACE ===") << std::endl;
			for (size_t i = 0; i < exception.stacktrace.size(); i++)
			{
				file << ("[") << i << ("] 0x") << std::hex << exception.stacktrace[i].address << std::dec << std::endl;
			}

			file << ("\n=== REGISTERS ===") << std::endl;
			for (const auto& reg : exception.registers)
			{
				file << reg.first << (": 0x") << std::hex << reg.second << std::dec << std::endl;
			}

			file.close();
		}

		char message[512];
		sprintf_s(message, ("Application crashed!\n\nException: %s (0x%X)\n\nCrash log saved to: %s"), exception.exceptionMetadata.exceptionName.c_str(), exception.exceptionMetadata.exceptionCode, filename);

		MessageBoxA(NULL, message, ("Crash Detected"), MB_OK | MB_ICONERROR);

		return EXCEPTION_EXECUTE_HANDLER;
	}
public:
	static void init()
	{
		crashlog::initialize();
		SetUnhandledExceptionFilter(exception_filter);
	}

	static void cleanup()
	{
		SetUnhandledExceptionFilter(nullptr);
	}
};