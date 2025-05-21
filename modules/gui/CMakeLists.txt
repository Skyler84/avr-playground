add_module(gui)
target_sources(gui
  PRIVATE
  encoder.c
  buttons.c
  gui.c
)

target_include_directories(gui
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_modules(gui
  PRIVATE
  gfx
)

target_link_libraries(gui
  PRIVATE
  module
  display
  fs
  font
)
