set(XWIFT_HTTP_SOURCES
  ${CMAKE_SOURCE_DIR}/lib/stdlib/HTTP/HTTPBackend.cpp
  ${CMAKE_SOURCE_DIR}/lib/stdlib/HTTP/HTTPPlugin.cpp
  ${CMAKE_SOURCE_DIR}/lib/stdlib/HTTP/URLParser.cpp
  ${CMAKE_SOURCE_DIR}/lib/stdlib/HTTP/BodyEncoder.cpp
)

add_library(XWiftHTTP STATIC ${XWIFT_HTTP_SOURCES})

target_include_directories(XWiftHTTP PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(XWiftHTTP PUBLIC XWiftBasic)
