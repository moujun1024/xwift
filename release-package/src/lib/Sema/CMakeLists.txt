set(XWIFT_SEMA_SOURCES
  ${CMAKE_SOURCE_DIR}/lib/Sema/Sema.cpp
)

add_library(XWiftSema STATIC ${XWIFT_SEMA_SOURCES})

target_include_directories(XWiftSema PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(XWiftSema PUBLIC XWiftAST XWiftParser)
