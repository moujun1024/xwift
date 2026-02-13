add_library(xwift_plugin STATIC
  Plugin.cpp
)

target_include_directories(xwift_plugin PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

target_link_libraries(xwift_plugin PUBLIC
  xwift_basic
)

if(WIN32)
  target_link_libraries(xwift_plugin PUBLIC
    ${CMAKE_SOURCE_DIR}/build/lib/xwift_plugin.lib
  )
endif()

add_library(xwift_http_plugin SHARED
  ../stdlib/HTTP/HTTPPlugin.cpp
  ../stdlib/HTTP/Win32Backend.cpp
)

target_include_directories(xwift_http_plugin PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

target_link_libraries(xwift_http_plugin PUBLIC
  xwift_plugin
  xwift_basic
)

if(WIN32)
  target_link_libraries(xwift_http_plugin PUBLIC
    winhttp
  )
endif()

add_library(xwift_json_plugin SHARED
  ../stdlib/JSON/JSONPlugin.cpp
  ../stdlib/JSON/JSON.cpp
)

target_include_directories(xwift_json_plugin PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

target_link_libraries(xwift_json_plugin PUBLIC
  xwift_plugin
  xwift_basic
)

if(WIN32)
  set_target_properties(xwift_http_plugin PROPERTIES
    SUFFIX ".dll"
    OUTPUT_NAME "libhttp_plugin"
  )
  
  set_target_properties(xwift_json_plugin PROPERTIES
    SUFFIX ".dll"
    OUTPUT_NAME "libjson_plugin"
  )
else()
  set_target_properties(xwift_http_plugin PROPERTIES
    SUFFIX ".so"
    OUTPUT_NAME "libhttp_plugin"
  )
  
  set_target_properties(xwift_json_plugin PROPERTIES
    SUFFIX ".so"
    OUTPUT_NAME "libjson_plugin"
  )
endif()
