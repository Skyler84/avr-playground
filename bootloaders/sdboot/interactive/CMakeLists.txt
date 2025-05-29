add_module(sdboot_interactive)
target_sources(sdboot_interactive
  PRIVATE
  interactive.c
)

target_include_directories(sdboot_interactive
  PRIVATE
  .
  PUBLIC
  include/
)

target_link_modules(sdboot_interactive
  PRIVATE
  lcd
  gfx
  font5x7
  buttons
  gui
  module
  sd
  fat
  boot
)

target_link_libraries(sdboot_interactive
  PRIVATE
  
  # mod_interface_lcd
  # mod_interface_gfx
  # mod_interface_font5x7
  # mod_interface_buttons
  # mod_interface_gui
  # mod_interface_module
  # mod_interface_sd
)