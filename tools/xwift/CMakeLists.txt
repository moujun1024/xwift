set(XWIFT_SOURCES
  ${CMAKE_SOURCE_DIR}/tools/xwift/xwift.cpp
)

add_executable(xwift ${XWIFT_SOURCES})

target_include_directories(xwift PRIVATE
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(xwift PRIVATE
  XWiftFrontend
  XWiftParser
  XWiftLexer
  XWiftBasic
  XWiftHTTP
  XWiftJSON
)
