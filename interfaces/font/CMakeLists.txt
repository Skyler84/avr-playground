add_library(font INTERFACE)
target_include_directories(font INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_definitions(font INTERFACE
    font_MODTYPE=INTERFACE
)
target_link_libraries(font INTERFACE
    display
)