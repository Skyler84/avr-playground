add_module(vfs)

target_sources(vfs
  PRIVATE
    vfs.c
)

target_include_directories(vfs
  PUBLIC
    include/
)

target_link_modules(vfs
    module
    
)

target_link_libraries(vfs
  PRIVATE
  fs
)