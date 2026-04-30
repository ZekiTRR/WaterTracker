# About the Project
An application for tracking daily water intake. It allows the user to track how much water they have drunk during the day and set water consumption goals.

# Technologies
The application is written in C++ using the Qt graphics library.

# Installation

## Downloading the Ready-Made Application
- You can download the ready-made application from Releases.

## Building from Source Code
The project uses the CMake build system.
Also note that Qt and CMake must be installed to build the project.
The path to Qt is specified in `CMakeLists.txt` on line 9: `set(CMAKE_PREFIX_PATH "E:/Qt/6.11.0/mingw_64/lib/cmake"`.

Example paths to CMake and Ninja from CLion:

```powershell
$cmake = "C:\Tools\CLion\bin\cmake\win\x64\bin\cmake.exe"
$ninja = "C:/Tools/CLion/bin/ninja/win/x64/ninja.exe"
```

In PowerShell, a quoted string by itself is treated as text. The `&` operator runs the specified path as a program.

### Release

Configure Release:

```powershell
& $cmake -S . -B cmake-build-release -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_MAKE_PROGRAM="$ninja"
```

Build Release:

```powershell
& $cmake --build cmake-build-release --config Release
```

### Debug

Configure Debug:

```powershell
& $cmake -S . -B cmake-build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MAKE_PROGRAM="$ninja"
```

Build Debug:

```powershell
& $cmake --build cmake-build-debug --config Debug
```

---
The description is still incomplete because the project is under development. More information about the application's functionality and usage instructions is planned for the future.
