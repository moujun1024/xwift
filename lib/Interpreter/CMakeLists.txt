set(XWIFT_INTERPRETER_SOURCES
  ${CMAKE_SOURCE_DIR}/lib/Interpreter/Interpreter.cpp
)

add_library(XWiftInterpreter STATIC ${XWIFT_INTERPRETER_SOURCES})

target_include_directories(XWiftInterpreter PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(XWiftInterpreter PUBLIC XWiftBasic XWiftAST)
