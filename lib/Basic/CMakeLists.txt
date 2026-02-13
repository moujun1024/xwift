set(XWIFT_BASIC_SOURCES
  ${CMAKE_SOURCE_DIR}/lib/Basic/LLVM.cpp
  ${CMAKE_SOURCE_DIR}/lib/Basic/Version.cpp
)

add_library(XWiftBasic STATIC ${XWIFT_BASIC_SOURCES})

target_include_directories(XWiftBasic PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)
