set(XWIFT_FRONTEND_SOURCES
  ${CMAKE_SOURCE_DIR}/lib/Frontend/Frontend.cpp
)

add_library(XWiftFrontend STATIC ${XWIFT_FRONTEND_SOURCES})

target_include_directories(XWiftFrontend PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(XWiftFrontend PUBLIC XWiftAST XWiftParser XWiftSema XWiftCodeGen)
