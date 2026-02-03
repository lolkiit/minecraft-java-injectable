@echo off
setlocal enabledelayedexpansion

echo   Nexus Client - Build Script
echo.

if not exist "gradlew.bat" (
    echo [INFO] Gradle wrapper not found, generating...
    call gradle wrapper --gradle-version 7.6
    if errorlevel 1 (
        echo [ERROR] Failed to generate Gradle wrapper
        pause
        exit /b 1
    )
)

echo [INFO] Cleaning previous build...
if exist "build" rmdir /s /q "build"

echo [INFO] Building project...
call gradlew.bat clean build --no-daemon
if errorlevel 1 (
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

if not exist "build\output\nexus-client.jar" (
    echo [ERROR] JAR file not found!
    pause
    exit /b 1
)

echo.
echo [SUCCESS] Build completed successfully!

echo.
echo [INFO] Converting JAR to C++ header...
python jar2header.py build\output\nexus-client.jar ..\cpp\mapper\src\common\data.h
if errorlevel 1 (
    echo [WARNING] Failed to convert JAR to header
    echo You may need to run jar2header.py manually
) else (
    echo [SUCCESS] JAR converted to C++ header successfully!
)

echo.
echo [INFO] Build process completed!
pause
