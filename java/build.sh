#!/bin/bash

echo "  Nexus Client - Build Script"
echo ""

if [ ! -f "gradlew" ]; then
    echo "[INFO] Gradle wrapper not found, generating..."
    gradle wrapper --gradle-version 7.6
    if [ $? -ne 0 ]; then
        echo "[ERROR] Failed to generate Gradle wrapper"
        exit 1
    fi
    chmod +x gradlew
fi

echo "[INFO] Cleaning previous build..."
rm -rf build

echo "[INFO] Building project..."
./gradlew clean build --no-daemon
if [ $? -ne 0 ]; then
    echo "[ERROR] Build failed!"
    exit 1
fi

if [ ! -f "build/output/nexus-client.jar" ]; then
    echo "[ERROR] JAR file not found!"
    exit 1
fi

echo ""
echo "[SUCCESS] Build completed successfully!"

echo ""
echo "[INFO] Converting JAR to C++ header..."
python3 jar2header.py build/output/nexus-client.jar ../cpp/mapper/src/common/data.h
if [ $? -ne 0 ]; then
    echo "[WARNING] Failed to convert JAR to header"
    echo "You may need to run jar2header.py manually"
else
    echo "[SUCCESS] JAR converted to C++ header successfully!"
fi

echo ""
echo "[INFO] Build process completed!"
