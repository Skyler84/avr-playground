add_library(ld_debug STATIC)
target_sources(ld_debug
    PRIVATE
        debug.c
)

target_compile_options(ld_debug
    PRIVATE
        -Os
)

target_link_libraries(ld_debug
    PRIVATE
        module
)