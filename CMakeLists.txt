cmake_minimum_required(VERSION 3.12)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(zombies)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(MACOSX TRUE)
endif()

set(CMAKE_CXX_STANDARD 20)

# Add the source files
file(GLOB SOURCES src/*.cpp)

# Add the include directory
include_directories(include)
include_directories(vendor/include)

if (WIN32)
  link_directories(vendor/vs)
  file(GLOB LIBRARIES "vendor/vs/*.lib")
  file(GLOB LIBRARIES "vendor/vs/*.dll")
  link_libraries(kgfx kgfx_gh glfw3)
# link_libraries(d3d12.lib dxgi.lib dxguid.lib d3dcompiler.lib)
elseif (MACOSX)
  link_directories(vendor/macos)
  file(GLOB LIBRARIES "vendor/macos/*.dylib")
  link_libraries(kgfx kgfx_gh glfw)
endif()

# Create the executable
add_executable(zombies ${SOURCES})
target_link_libraries(zombies ${LIBRARIES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET zombies PROPERTY CXX_STANDARD 20)
endif()
