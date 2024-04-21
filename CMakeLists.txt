cmake_minimum_required(VERSION 3.12)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(zombies)

set(CMAKE_CXX_STANDARD 20)

# Add the source files
file(GLOB SOURCES src/*.cpp)

# Add the include directory
include_directories(include)
include_directories(vendor/include)

link_directories(vendor/vs)
link_libraries(kgfx.lib kgfx_gh.lib glfw3.lib)
link_libraries(d3d12.lib dxgi.lib dxguid.lib d3dcompiler.lib)

# Create the executable
add_executable(zombies ${SOURCES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET zombies PROPERTY CXX_STANDARD 20)
endif()