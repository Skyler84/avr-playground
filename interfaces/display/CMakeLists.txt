add_library(display INTERFACE)
target_include_directories(display INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_definitions(display INTERFACE
    display_MODTYPE=INTERFACE
)