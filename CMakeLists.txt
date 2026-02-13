cmake_minimum_required(VERSION 3.20)
project(XWift)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(XWIFT_VERSION_MAJOR 0)
set(XWIFT_VERSION_MINOR 1)
set(XWIFT_VERSION_PATCH 0)
set(XWIFT_VERSION "${XWIFT_VERSION_MAJOR}.${XWIFT_VERSION_MINOR}.${XWIFT_VERSION_PATCH}")

option(BUILD_TESTING "Build and run tests" ON)
option(BUILD_PLUGINS "Build plugin libraries" ON)
option(XWIFT_ENABLE_PYTHON "Enable Python FFI support" OFF)
option(XWIFT_ENABLE_OBJC "Enable Objective-C FFI support" OFF)

message(STATUS "XWift Version: ${XWIFT_VERSION}")

include(CMakeDependentOption)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(lib/Basic)
add_subdirectory(lib/Lexer)
add_subdirectory(lib/AST)
add_subdirectory(lib/Parser)
add_subdirectory(lib/Interpreter)
add_subdirectory(lib/Sema)
add_subdirectory(lib/CodeGen)
add_subdirectory(lib/Frontend)
add_subdirectory(lib/Filesystem)
add_subdirectory(lib/Logging)
add_subdirectory(lib/stdlib)
add_subdirectory(lib/FFI)
add_subdirectory(lib/UI)

if(BUILD_PLUGINS)
  add_subdirectory(lib/Plugin)
endif()

add_subdirectory(tools/xwift)

if(BUILD_TESTING)
  enable_testing()
  add_subdirectory(test)
  add_subdirectory(unittests)
endif()
