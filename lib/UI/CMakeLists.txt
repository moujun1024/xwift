# XWift UI Library

add_library(xwift_ui
    UI.cpp
)

target_include_directories(xwift_ui
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(xwift_ui
    PUBLIC
    xwift_basic
)

if(WIN32)
    target_link_libraries(xwift_ui
        PUBLIC
        comctl32
        gdi32
        user32
    )
endif()

# Install UI library
install(TARGETS xwift_ui
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install UI headers
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/xwift/UI/
    DESTINATION include/xwift/UI
    FILES_MATCHING PATTERN "*.h"
)
