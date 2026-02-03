/*
	* ==================================== *
		* Project: Nexus Client (Mapper)
		* File: mapper/src/handler/console.h
		* Purpose: Debug console. AllocConsole, redirect stdout/stderr, LOG macro with timestamp.
		* Author: lolkit
	* ==================================== *
*/

#pragma once
#include <Windows.h>
#include <string>
#include <cstdarg>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>

class console_handler {
private:
public:
	static void init()
	{
		AllocConsole();

		FILE* dummy;
		freopen_s(&dummy, ("CONOUT$"), ("w"), stdout);
		freopen_s(&dummy, ("CONIN$"), ("r"), stdin);
		freopen_s(&dummy, ("CONOUT$"), ("w"), stderr);

		SetConsoleTitleA("Nexus Client");
	}

	static void log(const char* type, const char* format, ...)
	{
		SYSTEMTIME st;
		GetLocalTime(&st);

		printf(("[%02d:%02d:%02d] [%s] "), st.wHour, st.wMinute, st.wSecond, type);

		va_list args;
		va_start(args, format);
		vprintf(format, args);
		va_end(args);

		printf(("\n"));
	}

	static void cleanup()
	{
		FreeConsole();
	}
};

#define LOG(type, message, ...) console_handler::log(type, message, __VA_ARGS__)