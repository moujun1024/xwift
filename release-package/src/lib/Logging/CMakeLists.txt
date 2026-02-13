set(XWIFT_LOGGING_SOURCES
  ${CMAKE_SOURCE_DIR}/lib/Logging/Logger.cpp
)

add_library(XWiftLogging STATIC ${XWIFT_LOGGING_SOURCES})

target_include_directories(XWiftLogging PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(XWiftLogging PUBLIC XWiftBasic)
