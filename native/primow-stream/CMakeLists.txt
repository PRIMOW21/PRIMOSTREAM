cmake_minimum_required(VERSION 3.28)
project(PrimowStream VERSION 0.1.0 LANGUAGES CXX)
if(NOT WIN32)
  message(FATAL_ERROR "PRIMOW STREAM requires Windows x64 and MSVC.")
endif()
if(NOT TARGET OBS::libobs)
  message(FATAL_ERROR "Use scripts/build-windows.ps1 to build with pinned libobs.")
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_add_executable(primow-stream WIN32
  src/main.cpp
  src/capture/Windows.cpp src/capture/Windows.hpp
  src/security/Credentials.cpp src/security/Credentials.hpp
  src/streaming/Engine.cpp src/streaming/Engine.hpp
  src/ui/MainWindow.cpp src/ui/MainWindow.hpp)
target_compile_definitions(primow-stream PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN UNICODE _UNICODE)
target_compile_options(primow-stream PRIVATE /W4 /utf-8)
target_link_libraries(primow-stream PRIVATE Qt6::Widgets OBS::libobs advapi32 dwmapi)
install(TARGETS primow-stream RUNTIME DESTINATION bin/64bit)
