cmake_minimum_required(VERSION 3.20)
project(XWiftStdLib)

include_directories(${CMAKE_SOURCE_DIR}/include)

# Collections library
set(COLLECTIONS_SOURCES
  Collections/Array.cpp
  Collections/Dictionary.cpp
  Collections/Set.cpp
)

# Error handling library
set(ERROR_SOURCES
  Error/Error.cpp
)

# Concurrency library
set(CONCURRENCY_SOURCES
  Concurrency/Async.cpp
)

# Filesystem library
set(FILESYSTEM_SOURCES
  Filesystem/Path.cpp
)

# JSON library
set(JSON_SOURCES
  JSON/JSON.cpp
)

# HTTP library
set(HTTP_SOURCES
  HTTP/HTTPClient.cpp
  HTTP/URLParser.cpp
  HTTP/BodyEncoder.cpp
)

# Terminal library
set(TERMINAL_SOURCES
  Terminal/Terminal.cpp
)

# Create libraries
add_library(XWiftCollections STATIC ${COLLECTIONS_SOURCES})
add_library(XWiftError STATIC ${ERROR_SOURCES})
add_library(XWiftConcurrency STATIC ${CONCURRENCY_SOURCES})
add_library(XWiftFilesystem STATIC ${FILESYSTEM_SOURCES})

add_library(XWiftJSON STATIC ${JSON_SOURCES})
add_library(XWiftHTTP STATIC ${HTTP_SOURCES})
add_library(XWiftTerminal STATIC ${TERMINAL_SOURCES})

# JSON client library
set(JSON_CLIENT_SOURCES
  JSON/JSONClient.cpp
)

add_library(XWiftJSONClient STATIC ${JSON_CLIENT_SOURCES})

# Platform-specific HTTP backend
if(WIN32)
  set(HTTP_BACKEND_SOURCES
    HTTP/Win32Backend.cpp
  )
  add_library(XWiftHTTPBackend STATIC ${HTTP_BACKEND_SOURCES})
  target_link_libraries(XWiftHTTPBackend winhttp)
else()
  set(HTTP_BACKEND_SOURCES
    HTTP/CurlBackend.cpp
  )
  add_library(XWiftHTTPBackend STATIC ${HTTP_BACKEND_SOURCES})
  find_package(CURL REQUIRED)
  target_include_directories(XWiftHTTPBackend PRIVATE
    ${CURL_INCLUDE_DIRS}
  )
  target_link_libraries(XWiftHTTPBackend ${CURL_LIBRARIES})
endif()

# Link libraries
target_link_libraries(XWiftHTTP
  XWiftHTTPBackend
)

target_link_libraries(XWiftJSONClient
  XWiftJSON
)

target_link_libraries(XWiftTerminal
  XWiftCollections
  XWiftError
  XWiftConcurrency
  XWiftFilesystem
)

target_link_libraries(XWiftJSON
  XWiftCollections
  XWiftError
)

# Include directories
target_include_directories(XWiftHTTP PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_include_directories(XWiftJSON PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_include_directories(XWiftJSONClient PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_include_directories(XWiftTerminal PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_include_directories(XWiftCollections PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_include_directories(XWiftError PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_include_directories(XWiftConcurrency PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_include_directories(XWiftFilesystem PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)
