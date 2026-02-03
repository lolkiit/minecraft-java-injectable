/*
	* ==================================== *
		* Project: Nexus Client (Loader)
		* File: loader/src/main.cpp
		* Purpose: Process injector. Finds Minecraft (javaw.exe), injects mapper.dll into the process.
		* Author: lolkit
	* ==================================== *
*/

#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include <handler/log.h>
#include <handler/process.h>

static std::wstring getExecutablePath()
{
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    std::wstring path(buffer);
    size_t lastSlash = path.find_last_of(L"\\/");
    return (lastSlash != std::wstring::npos) ? path.substr(0, lastSlash) : L"";
}

int main()
{
    using namespace loader;

    std::wstring mapperName = L"mapper.dll";
    std::wstring processName = L"javaw.exe";

    log_handler::info("Searching for Minecraft process...");

    std::vector<DWORD> processIds = process_handler::findAllProcessesByName(processName);
    if (processIds.empty())
    {
        log_handler::error("Minecraft process not found!");
        std::cin.get();
        return 1;
    }

    DWORD targetProcessId = processIds[0];
    if (processIds.size() > 1)
    {
        log_handler::warning("Multiple Minecraft instances found");
        log_handler::info("Select process ID:");

        for (size_t i = 0; i < processIds.size(); ++i)
        {
            std::cout << "  [" << i + 1 << "] PID: " << processIds[i] << std::endl;
        }

        int choice;
        std::cout << "Choice: ";
        std::cin >> choice;

        if (choice < 1 || choice > static_cast<int>(processIds.size()))
        {
            log_handler::error("Invalid choice");
            return 1;
        }

        targetProcessId = processIds[choice - 1];
    }

    log_handler::success("Found Minecraft process (PID: " + std::to_string(targetProcessId) + ")");

    std::wstring mapperPath = getExecutablePath() + L"\\" + mapperName;
    
    if (GetFileAttributesW(mapperPath.c_str()) == INVALID_FILE_ATTRIBUTES) 
    {
        log_handler::error("Mapper DLL not found: " + std::string(mapperPath.begin(), mapperPath.end()));
        std::cout << std::endl << "Press Enter to exit...";
        std::cin.get();
        return 1;
    }

    log_handler::info("Mapper DLL: " + std::string(mapperPath.begin(), mapperPath.end()));
    log_handler::info("Starting injection...");

    injector_handler injector(targetProcessId, mapperPath);
    if (injector.inject()) 
    {
        log_handler::success("Successfully injected Nexus Client!");
    }
    else 
    {
        log_handler::error("Injection failed!");
    }

    std::cin.get();

    return 0;
}
