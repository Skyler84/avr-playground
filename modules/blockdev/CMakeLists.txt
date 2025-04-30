add_module(blockdev)
target_sources(blockdev
    PRIVATE
        blockdev.c
)
target_include_directories(blockdev
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_modules(blockdev
    PRIVATE
        module
)