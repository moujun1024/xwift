set(XWIFT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT DEFINED XWIFT_INSTALL_PREFIX)
  set(XWIFT_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/xwift")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

option(XWIFT_ENABLE_WERROR "Treat warnings as errors" OFF)
option(XWIFT_ENABLE_EXPERIMENTAL "Enable experimental features" OFF)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(XWIFT_ENABLE_ASSERTIONS ON)
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  set(XWIFT_ENABLE_ASSERTIONS ON)
else()
  set(XWIFT_ENABLE_ASSERTIONS OFF)
endif()

if(MSVC)
  add_compile_options(/W4)
  if(XWIFT_ENABLE_WERROR)
    add_compile_options(/WX)
  endif()
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
  if(XWIFT_ENABLE_WERROR)
    add_compile_options(-Werror)
  endif()
endif()

function(xwift_add_library name)
  add_library(${name} ${ARGN})
  target_include_directories(${name} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  )
  set_target_properties(${name} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
  )
endfunction()

function(xwift_add_executable name)
  add_executable(${name} ${ARGN})
  target_include_directories(${name} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  )
endfunction()
