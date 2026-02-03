/*
	* ==================================== *
		* Project: Nexus Client (Loader)
		* File: loader/src/handler/process.h
		* Purpose: Process enumeration and injection helpers. Finds processes by name, opens handle, performs DLL injection.
		* Author: lolkit
	* ==================================== *
*/

#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>

#include "log.h"

namespace loader 
{
    class process_handler {
    public:
        static DWORD findProcessByName(const std::wstring& processName) 
        {
            HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (snapshot == INVALID_HANDLE_VALUE) 
            {
                return 0;
            }

            PROCESSENTRY32W entry = { sizeof(PROCESSENTRY32W) };
            if (Process32FirstW(snapshot, &entry)) 
            {
                do {
                    if (processName == entry.szExeFile) 
                    {
                        CloseHandle(snapshot);
                        return entry.th32ProcessID;
                    }
                } while (Process32NextW(snapshot, &entry));
            }

            CloseHandle(snapshot);
            return 0;
        }

        static std::vector<DWORD> findAllProcessesByName(const std::wstring& processName) 
        {
            std::vector<DWORD> processIds;
            HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (snapshot == INVALID_HANDLE_VALUE) 
            {
                return processIds;
            }

            PROCESSENTRY32W entry = { sizeof(PROCESSENTRY32W) };
            if (Process32FirstW(snapshot, &entry))
            {
                do {
                    if (processName == entry.szExeFile) 
                    {
                        processIds.push_back(entry.th32ProcessID);
                    }
                } while (Process32NextW(snapshot, &entry));
            }

            CloseHandle(snapshot);
            return processIds;
        }

        static bool isProcessRunning(DWORD processId) 
        {
            HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
            if (!process)
            {
                return false;
            }

            DWORD exitCode;
            bool running = GetExitCodeProcess(process, &exitCode) && exitCode == STILL_ACTIVE;
            CloseHandle(process);
            return running;
        }
    };

    class injector_handler {
    private:
        DWORD m_processId;
        std::wstring m_dllPath;
    public:
        injector_handler(DWORD processId, const std::wstring& dllPath) : m_processId(processId), m_dllPath(dllPath) {}

        bool inject()
        {
            if (!process_handler::isProcessRunning(m_processId))
            {
                log_handler::error("Target process is not running");
                return false;
            }

            HANDLE process = OpenProcess(
                PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
                PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
                FALSE,
                m_processId
            );

            if (!process) 
            {
                log_handler::error("Failed to open target process (Error: " + std::to_string(GetLastError()) + ")");
                return false;
            }

            bool result = injectDll(process);
            CloseHandle(process);
            return result;
        }

    private:
        bool injectDll(HANDLE process) 
        {
            size_t pathSize = (m_dllPath.length() + 1) * sizeof(wchar_t);

            LPVOID remotePath = VirtualAllocEx(
                process,
                nullptr,
                pathSize,
                MEM_COMMIT | MEM_RESERVE,
                PAGE_READWRITE
            );

            if (!remotePath) 
            {
                log_handler::error("Failed to allocate memory in target process");
                return false;
            }

            if (!WriteProcessMemory(process, remotePath, m_dllPath.c_str(), pathSize, nullptr)) 
            {
                log_handler::error("Failed to write DLL path to target process");
                VirtualFreeEx(process, remotePath, 0, MEM_RELEASE);
                return false;
            }

            HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
            if (!kernel32) 
            {
                log_handler::error("Failed to get kernel32.dll handle");
                VirtualFreeEx(process, remotePath, 0, MEM_RELEASE);
                return false;
            }

            LPVOID loadLibraryW = (LPVOID)GetProcAddress(kernel32, "LoadLibraryW");
            if (!loadLibraryW)
            {
                log_handler::error("Failed to get LoadLibraryW address");
                VirtualFreeEx(process, remotePath, 0, MEM_RELEASE);
                return false;
            }

            HANDLE thread = CreateRemoteThread(
                process,
                nullptr,
                0,
                (LPTHREAD_START_ROUTINE)loadLibraryW,
                remotePath,
                0,
                nullptr
            );

            if (!thread)
            {
                log_handler::error("Failed to create remote thread");
                VirtualFreeEx(process, remotePath, 0, MEM_RELEASE);
                return false;
            }

            WaitForSingleObject(thread, INFINITE);

            DWORD exitCode;
            GetExitCodeThread(thread, &exitCode);

            CloseHandle(thread);
            VirtualFreeEx(process, remotePath, 0, MEM_RELEASE);

            if (exitCode == 0)
            {
                log_handler::error("LoadLibraryW returned NULL");
                return false;
            }

            return true;
        }
    };
}
