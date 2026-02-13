set(XWIFT_FILESYSTEM_SOURCES
  ${CMAKE_SOURCE_DIR}/lib/Filesystem/Filesystem.cpp
)

add_library(XWiftFilesystem STATIC ${XWIFT_FILESYSTEM_SOURCES})

target_include_directories(XWiftFilesystem PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(XWiftFilesystem PUBLIC XWiftBasic)
