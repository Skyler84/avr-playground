add_module(font5x7)
target_sources(font5x7
    PRIVATE
        font5x7.c
)
target_include_directories(font5x7
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_modules(font5x7
    PRIVATE
        module
)
target_link_libraries(font5x7
    PUBLIC
        font
)