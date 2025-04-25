add_library(blockdev INTERFACE)
target_include_directories(blockdev INTERFACE include)
target_link_modules(blockdev
    INTERFACE
        blockdev
)