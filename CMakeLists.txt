cmake_minimum_required(VERSION 3.26)

cmake_policy(SET CMP0091 NEW)

project(VerletSFML)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(VCPKG_TARGET_TRIPLET MATCHES "-static$")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

find_package(glm REQUIRED)
find_package(SFML 2.5.0 COMPONENTS system window graphics REQUIRED)
find_package(ImGui-SFML REQUIRED)

set(SOURCES src/Main.cpp src/Application.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE include)

target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)
target_link_libraries(${PROJECT_NAME} PRIVATE sfml-system sfml-window sfml-graphics)
target_link_libraries(${PROJECT_NAME} PRIVATE ImGui-SFML::ImGui-SFML)
