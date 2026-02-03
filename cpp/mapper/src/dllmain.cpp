/*
	* ==================================== *
		* Project: Nexus Client (Mapper)
		* File: mapper/src/dllmain.cpp
		* Purpose: DLL entry point. Initializes console, JNI, loads embedded JAR and invokes Java entry point.
		* Author: lolkit
	* ==================================== *
*/

#include <windows.h>
#include <thread>
#include <chrono>

#include "handler/console.h"
#include "handler/exception.h"
#include "handler/hook.h"
#include "handler/jni.h"

#include "core/jar_loader.h"

#include "common/data.h"

void MainThread(HMODULE hModule)
{
	console_handler::init();
	exception_handler::init();

	if (!jni_handler::init())
	{
		LOG("ERROR", "Failed to initialize JNI");
		MessageBoxA(nullptr, "Failed to initialize JNI\nMake sure you're running Minecraft", "Nexus Client Error", MB_OK | MB_ICONERROR);
		goto cleanup;
	}

	LOG("SUCCESS", "JNI initialized successfully");

	{
		nexus::jar_loader loader;
		if (!loader.load_from_memory(nexus::data::JAR_BYTES, nexus::data::JAR_SIZE))
		{
			LOG("ERROR", "Failed to load JAR");
			MessageBoxA(nullptr, "Failed to load JAR file", "Nexus Client Error", MB_OK | MB_ICONERROR);
			goto cleanup;
		}

		LOG(("SUCCESS"), ("JAR loaded successfully"));

		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		LOG(("INFO"), ("Invoking entry point..."));

		if (!loader.invoke_entry_point("com.nexus.client.injection.EntryPoint", "init"))
		{
			LOG(("ERROR"), ("Failed to invoke entry point"));
			MessageBoxA(nullptr, "Failed to invoke entry point", "Nexus Client Error", MB_OK | MB_ICONERROR);
			goto cleanup;
		}

		LOG(("SUCCESS"), ("Nexus Client loaded successfully!"));
	}

	return;

cleanup:
	LOG(("INFO"), ("Cleaning up..."));
	
	jni_handler::cleanup();
	exception_handler::cleanup();
	
	std::this_thread::sleep_for(std::chrono::seconds(5));
	console_handler::cleanup();
	
	FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	DisableThreadLibraryCalls(hModule);

	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			CloseHandle(CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(MainThread), hModule, 0, nullptr));
			break;
		}
	}

	return TRUE;
}