cmake_minimum_required(VERSION 3.20)

add_library(xwift_ffi STATIC
    FFI.cpp
    Python/PythonFFI.cpp
    ObjC/ObjCFFI.cpp
)

target_include_directories(xwift_ffi PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

target_link_libraries(xwift_ffi PUBLIC
    xwift_basic
)

if(UNIX)
    target_link_libraries(xwift_ffi PUBLIC
        dl
    )
endif()

if(XWIFT_ENABLE_PYTHON)
    find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
    target_include_directories(xwift_ffi PUBLIC
        ${Python3_INCLUDE_DIRS}
    )
    target_link_libraries(xwift_ffi PUBLIC
        ${Python3_LIBRARIES}
    )
    target_compile_definitions(xwift_ffi PUBLIC XWIFT_ENABLE_PYTHON)
endif()

if(XWIFT_ENABLE_OBJC)
    find_library(FOUNDATION_LIBRARY Foundation REQUIRED)
    find_library(OBJC_LIBRARY objc REQUIRED)
    target_link_libraries(xwift_ffi PUBLIC
        ${FOUNDATION_LIBRARY}
        ${OBJC_LIBRARY}
    )
    target_compile_definitions(xwift_ffi PUBLIC XWIFT_ENABLE_OBJC)
endif()
