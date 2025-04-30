add_module(module)
target_sources(module
    PRIVATE
        module.c
)

target_include_directories(module
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)