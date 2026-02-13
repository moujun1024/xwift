set(XWIFT_PARSER_SOURCES
  ${CMAKE_SOURCE_DIR}/lib/Parser/Parser.cpp
  ${CMAKE_SOURCE_DIR}/lib/Parser/SyntaxParser.cpp
)

add_library(XWiftParser STATIC ${XWIFT_PARSER_SOURCES})

target_include_directories(XWiftParser PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(XWiftParser PUBLIC XWiftAST XWiftLexer XWiftInterpreter)
