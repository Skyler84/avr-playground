add_module(fonts)
target_sources(fonts
    PRIVATE
        fonts.c
        font5x7.c
)
target_include_directories(fonts
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_modules(fonts
    PRIVATE
        module
)