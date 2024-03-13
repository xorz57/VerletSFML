# VerletSFML

[![Build](https://github.com/xorz57/VerletSFML/actions/workflows/Build.yml/badge.svg)](https://github.com/xorz57/VerletSFML/actions/workflows/Build.yml)

![image](https://github.com/xorz57/VerletSFML/assets/84932056/78b7a699-0d83-4411-a54d-53938ae0a5b8)

## Changes
- [x] Enhanced CMakeLists.txt for improved project management.
- [x] Implemented vcpkg for streamlined dependency management.
- [x] Conducted thorough source code refactoring to enhance readability by eliminating unnecessary abstractions.
- [x] Addressed numerous static analysis issues to improve code quality.
- [x] Integrated glm library for comprehensive mathematical operations.
- [x] Utilized imgui-sfml for intuitive graphical user interface (GUI) development.
- [x] Introduced Application class for better organization and structure.
- [x] Separated tickrate from framerate for improved performance control.
- [x] Incorporated zoom functionality for enhanced user experience.

## Dependencies

- [SFML](https://github.com/SFML/SFML)
- [ImGui-SFML](https://github.com/SFML/imgui-sfml)

## How to Build

#### Linux & macOS

```bash
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
~/vcpkg/bootstrap-vcpkg.sh

git clone https://github.com/xorz57/VerletSFML.git
cd VerletSFML
cmake -B build -DCMAKE_BUILD_TYPE=Release -S . -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
ctest --build-config Release
```

#### Windows

```powershell
git clone https://github.com/microsoft/vcpkg.git C:/vcpkg
C:/vcpkg/bootstrap-vcpkg.bat
C:/vcpkg/vcpkg.exe integrate install

git clone https://github.com/xorz57/VerletSFML.git
cd VerletSFML
cmake -B build -DCMAKE_BUILD_TYPE=Release -S . -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
ctest --build-config Release
```
