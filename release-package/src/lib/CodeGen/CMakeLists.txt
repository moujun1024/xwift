set(XWIFT_CODEGEN_SOURCES
  ${CMAKE_SOURCE_DIR}/lib/CodeGen/CodeGen.cpp
)

add_library(XWiftCodeGen STATIC ${XWIFT_CODEGEN_SOURCES})

target_include_directories(XWiftCodeGen PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(XWiftCodeGen PUBLIC XWiftAST XWiftSema)
