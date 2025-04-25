add_library(fs INTERFACE)
target_include_directories(fs INTERFACE include)
target_link_libraries(fs
    INTERFACE
        blockdev
)