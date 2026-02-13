cmake_minimum_required(VERSION 3.20)
project(XWiftStdLib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(HTTP_SOURCES
  HTTP/HTTP.cpp
  HTTP/Win32Backend.cpp
)

set(JSON_SOURCES
  JSON/JSON.cpp
)

add_library(XWiftHTTP STATIC ${HTTP_SOURCES})
add_library(XWiftJSON STATIC ${JSON_SOURCES})

target_link_libraries(XWiftHTTP winhttp)

target_include_directories(XWiftHTTP PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_include_directories(XWiftJSON PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)
