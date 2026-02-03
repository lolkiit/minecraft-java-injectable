# Nexus Client

A professional Java injectable for Minecraft, built with clean architecture and modern C++ design patterns.

## Overview

Nexus Client is a Java-based Minecraft client injection system that loads compiled Java code directly into a running Minecraft process using JNI (Java Native Interface). Similar to other popular clients like Lunar/Badlion Client, it operates by:

1. **Loader**: Injects a DLL into the Minecraft process
2. **Mapper**: Loads the Java into the JVM using JNI
3. **Java Client**: Executes custom code within Minecraft's runtime

## Project Structure

```
nexus-client/
├── cpp/
│   ├── loader/              # Process injection tool
│   │   └── src/
│   │       └── main.cpp     # DLL injector implementation
│   └── mapper/              # JAR loader (DLL)
│       ├── src/
│       │   ├── core/        # JAR loading & class parsing
│       │   ├── handler/     # JNI, console, hooks, exceptions
│       │   └── dllmain.cpp  # DLL entry point
│       └── thirdparty/      # External dependencies
└── java/                    # Java client project
    ├── src/main/java/
    │   └── com/nexus/client/
    │       ├── BaseClient.java        # Main client class
    │       ├── handler/               # Handler utilities
    │       └── injection/             # Injection entry point
    ├── build.gradle         # Gradle configuration
    ├── build.bat            # Windows build script
    ├── build.sh             # Linux/Mac build script
    └── jar2header.py        # JAR to C++ converter
```

## Building

### Prerequisites

- **Java**: JDK 8 or higher
- **C++**: Visual Studio 2022+ (with C++20 support)
- **Python**: 3.x (for jar2header.py)
- **Gradle**: 7.6+ (downloaded automatically)

### Step 1: Build Java Project

#### Windows
```batch
cd java
build.bat
```

#### Linux/Mac
```bash
cd java
chmod +x build.sh
./build.sh
```

This will:
1. Compile the Java source code
2. Create `nexus-client.jar`
3. Convert JAR to C++ header (`jar_data.h`)

### Step 2: Build C++ Projects

Open `cpp/nexus-client.sln` in Visual Studio and build:
1. **mapper** (Release x64) - Creates `mapper.dll`
2. **loader** (Release x64) - Creates `loader.exe`

## Usage

### Running the Client

1. Start Minecraft (Vanilla or Forge)
2. Run `loader.exe`
3. Select the Minecraft process if multiple instances are running
4. The loader will inject `mapper.dll` into Minecraft
5. The mapper will load the Java client

## Third-Party Libraries

| Library | Purpose | License / Source |
|--------|---------|------------------|
| [miniz](https://github.com/richgel999/miniz) | ZIP/JAR extraction | Public domain |
| [MinHook](https://github.com/TsudaKageyu/minhook) | Function hooking | BSD 2-Clause |
| JDK (JNI) | Java Native Interface headers & `jvm.lib` | Oracle / OpenJDK |
| crashlog | Exception handling & stack traces | (see `cpp/mapper/thirdparty/`) |

- **miniz** is used in the mapper to extract `.class` files from the embedded JAR.
- **MinHook** is used for runtime function hooking (if enabled).
- **JNI** is required to load classes into the Minecraft JVM and invoke the entry point.

## Credits

- **lolkit** — creator & maintainer

---

**Built with ❤️ for the Minecraft modding community**
