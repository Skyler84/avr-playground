if (NOT "${BOARD}" STREQUAL "lafortuna")
    return()
endif()

add_module(buttons)
target_sources(buttons
    PRIVATE
        buttons.c
)
target_include_directories(buttons
    PRIVATE
        .
    include/
)

target_link_libraries(buttons
    PRIVATE
        module
)