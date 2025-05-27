add_executable(bootjacker)
target_sources(bootjacker
  PRIVATE
  main.c
  gui.c
  encoder.c
  buttons.c
  spm.c
)

target_include_directories(bootjacker
  PRIVATE
  .
)

target_link_modules(bootjacker
  PRIVATE
  # lcd
  # gfx
)
  
target_link_libraries(bootjacker
  PRIVATE
  module
  lcd
  gfx
  font5x7
  mod_interface_boot
  fat
  sd
)

set_target_properties(bootjacker PROPERTIES
  APP_MODULE_START_ADDRESS 0x8000
)

make_avr_app(bootjacker)

add_custom_target(bootjacker.app_flash_all_with_bootloader
  DEPENDS bootjacker.app_flash_all sdboot.boot.all.hex
  COMMAND avrdude -c c232hm -p usb1286 -D -U flash:w:${CMAKE_BINARY_DIR}/bootloaders/sdboot/sdboot.boot.all.hex
)

set_target_properties(bootjacker.app_flash_all_with_bootloader PROPERTIES
  FOLDER "Apps/bootjacker/Flash"
)
