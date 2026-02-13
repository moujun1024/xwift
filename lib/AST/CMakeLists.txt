set(XWIFT_AST_SOURCES
  ${CMAKE_SOURCE_DIR}/lib/AST/Decl.cpp
  ${CMAKE_SOURCE_DIR}/lib/AST/Type.cpp
)

add_library(XWiftAST STATIC ${XWIFT_AST_SOURCES})

target_include_directories(XWiftAST PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(XWiftAST PUBLIC XWiftBasic)
