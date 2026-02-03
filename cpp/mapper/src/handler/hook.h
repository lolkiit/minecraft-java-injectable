/*
	* ==================================== *
		* Project: Nexus Client (Mapper)
		* File: mapper/src/handler/hook.h
		* Purpose: MinHook wrapper. Initialize, create/enable/disable hooks.
		* Author: lolkit
	* ==================================== *
*/

#pragma once
#include <minhook/include/minhook.h>
#include "console.h"

class hook_handler {
private:
public:
	static bool init()
	{
		MH_STATUS status = MH_Initialize();
		if (status != MH_OK && status != MH_ERROR_ALREADY_INITIALIZED)
		{
			LOG(("ERROR"), ("Failed to initialize MinHook: %s"), MH_StatusToString(status));
			return false;
		}

		return true;
	}

	template<typename T>
	static bool create_hook(void* target, void* detour, T** original)
	{
		if (!target || !detour || !original)
		{
			LOG(("ERROR"), ("Invalid hook parameters"));
			return false;
		}

		MH_RemoveHook(target);

		MH_STATUS status;

		status = MH_CreateHook(target, detour, reinterpret_cast<void**>(original));
		if (status != MH_OK)
		{
			LOG(("ERROR"), ("Failed to create hook: %s"), MH_StatusToString(status));
			return false;
		}

		status = MH_EnableHook(target);
		if (status != MH_OK)
		{
			LOG(("ERROR"), ("Failed to enable hook: %s"), MH_StatusToString(status));
			MH_RemoveHook(target);
			return false;
		}

		return true;
	}

	static bool enable_hook(LPVOID target)
	{
		MH_STATUS status = MH_EnableHook(target);
		if (status != MH_OK)
		{
			LOG(("ERROR"), ("Failed to enable hook: %s"), MH_StatusToString(status));
			return false;
		}

		return true;
	}

	static bool disable_hook(LPVOID target)
	{
		MH_STATUS status;

		status = MH_DisableHook(target);
		if (status != MH_OK)
		{
			LOG(("ERROR"), ("Failed to disable hook: %s"), MH_StatusToString(status));
			return false;
		}

		status = MH_RemoveHook(target);
		if (status != MH_OK)
		{
			LOG(("ERROR"), ("Failed to remove hook: %s"), MH_StatusToString(status));
			return false;
		}

		return true;
	}

	static bool enable_all_hooks()
	{
		MH_STATUS status = MH_EnableHook(MH_ALL_HOOKS);
		if (status != MH_OK)
		{
			LOG(("ERROR"), ("Failed to enable all hooks: %s"), MH_StatusToString(status));
			return false;
		}

		return true;
	}

	static bool disable_all_hooks()
	{
		MH_STATUS status;
		status = MH_DisableHook(MH_ALL_HOOKS);
		if (status != MH_OK)
		{
			LOG(("ERROR"), ("Failed to disable all hooks: %s"), MH_StatusToString(status));
			return false;
		}

		status = MH_RemoveHook(MH_ALL_HOOKS);
		if (status != MH_OK)
		{
			LOG(("ERROR"), ("Failed to remove all hooks: %s"), MH_StatusToString(status));
			return false;
		}

		return true;
	}

	static void cleanup()
	{
		disable_all_hooks();
		MH_Uninitialize();
	}
};