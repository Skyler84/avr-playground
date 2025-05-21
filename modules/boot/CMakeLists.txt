add_module(boot)
target_sources(boot
  PRIVATE
  boot.c
)

target_include_directories(boot
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_modules(boot
  PRIVATE
  module
)

