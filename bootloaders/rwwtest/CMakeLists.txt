if(NOT "${BOARD}" STREQUAL "lafortuna")
  return()
endif()

add_executable(rwwtest)
target_sources(rwwtest
  PRIVATE
  main.c
)
target_include_directories(rwwtest
  PRIVATE
  .
)
target_link_modules(rwwtest
  PRIVATE
  lcd
  gfx
  font5x7
)
target_link_libraries(rwwtest
  PRIVATE
  module
)

set_target_properties(rwwtest PROPERTIES
  BOOT_START_ADDRESS 0x1E000
  BOOT_MODULE_START_ADDRESS 0x1c000
)

make_avr_bootloader(rwwtest)