set(XWIFT_LEXER_SOURCES
  ${CMAKE_SOURCE_DIR}/lib/Lexer/Lexer.cpp
)

add_library(XWiftLexer STATIC ${XWIFT_LEXER_SOURCES})

target_include_directories(XWiftLexer PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(XWiftLexer PUBLIC XWiftBasic)
