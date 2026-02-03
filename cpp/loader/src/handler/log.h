/*
	* ==================================== *
		* Project: Nexus Client (Loader)
		* File: loader/src/handler/log.h
		* Purpose: Console logging with colored output (INFO, SUCCESS, WARNING, ERROR).
		* Author: lolkit
	* ==================================== *
*/

#pragma once
#include <windows.h>
#include <string>
#include <iostream>

namespace loader 
{
    class log_handler {
    public:
        enum class Level {
            _INFO,
            _SUCCESS,
            _WARNING,
            _ERROR
        };

        static void log(Level level, const std::string& message) 
        {
            HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
            CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
            GetConsoleScreenBufferInfo(console, &consoleInfo);
            WORD originalColor = consoleInfo.wAttributes;

            switch (level) 
            {
            case Level::_INFO:
                SetConsoleTextAttribute(console, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
                std::cout << "[INFO] ";
                break;
            case Level::_SUCCESS:
                SetConsoleTextAttribute(console, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                std::cout << "[SUCCESS] ";
                break;
            case Level::_WARNING:
                SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                std::cout << "[WARNING] ";
                break;
            case Level::_ERROR:
                SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_INTENSITY);
                std::cout << "[ERROR] ";
                break;
            }

            SetConsoleTextAttribute(console, originalColor);
            std::cout << message << std::endl;
        }

        static void info(const std::string& message) { log(Level::_INFO, message); }
        static void success(const std::string& message) { log(Level::_SUCCESS, message); }
        static void warning(const std::string& message) { log(Level::_WARNING, message); }
        static void error(const std::string& message) { log(Level::_ERROR, message); }
    };
}