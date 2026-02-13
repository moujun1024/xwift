cmake_minimum_required(VERSION 3.20)
project(XWiftStdLib)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(JSON_SOURCES
  JSON/JSON.cpp
)

set(HTTP_SOURCES
  HTTP/HTTPClient.cpp
  HTTP/URLParser.cpp
  HTTP/BodyEncoder.cpp
)

set(JSON_CLIENT_SOURCES
  JSON/JSONClient.cpp
)

set(TERMINAL_SOURCES
  Terminal/Terminal.cpp
)

add_library(XWiftJSON STATIC ${JSON_SOURCES})
add_library(XWiftHTTP STATIC ${HTTP_SOURCES})
add_library(XWiftJSONClient STATIC ${JSON_CLIENT_SOURCES})

add_library(XWiftTerminal STATIC ${TERMINAL_SOURCES})

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

target_link_libraries(XWiftHTTP
  XWiftHTTPBackend
)

target_link_libraries(XWiftJSONClient
  XWiftJSON
)

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
