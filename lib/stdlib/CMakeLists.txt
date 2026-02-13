cmake_minimum_required(VERSION 3.20)
project(XWiftStdLib)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(JSON_SOURCES
  JSON/JSON.cpp
)

set(HTTP_CLIENT_SOURCES
  HTTP/HTTPClient.cpp
)

set(JSON_CLIENT_SOURCES
  JSON/JSONClient.cpp
)

add_library(XWiftJSON STATIC ${JSON_SOURCES})
add_library(XWiftHTTPClient STATIC ${HTTP_CLIENT_SOURCES})
add_library(XWiftJSONClient STATIC ${JSON_CLIENT_SOURCES})

if(WIN32)
  set(HTTP_SOURCES
    HTTP/Win32Backend.cpp
  )
  add_library(XWiftHTTP STATIC ${HTTP_SOURCES})
  target_link_libraries(XWiftHTTP winhttp)
else()
  set(HTTP_SOURCES
    HTTP/CurlBackend.cpp
  )
  add_library(XWiftHTTP STATIC ${HTTP_SOURCES})
  find_package(CURL REQUIRED)
  target_include_directories(XWiftHTTP PRIVATE
    ${CURL_INCLUDE_DIRS}
  )
  target_link_libraries(XWiftHTTP ${CURL_LIBRARIES})
endif()

target_link_libraries(XWiftHTTPClient
  XWiftHTTP
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

target_include_directories(XWiftHTTPClient PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_include_directories(XWiftJSONClient PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)
