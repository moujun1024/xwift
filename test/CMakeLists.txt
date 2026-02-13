set(XWIFT_TEST_SOURCES
  ${CMAKE_SOURCE_DIR}/test/test_framework.cpp
)

add_executable(XWiftTests ${XWIFT_TEST_SOURCES})

target_include_directories(XWiftTests PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(XWiftTests PUBLIC XWiftBasic XWiftJSON XWiftFilesystem)

add_test(NAME XWiftTests COMMAND XWiftTests)
