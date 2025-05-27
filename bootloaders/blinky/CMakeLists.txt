add_executable(blinky_boot)
target_sources(blinky_boot
  PRIVATE
  main.c
)
target_include_directories(blinky_boot
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(blinky_boot
  PRIVATE
  module
  boot
)

if ("${BOARD}" STREQUAL "lafortuna")
  set_target_properties(blinky_boot
    PROPERTIES
    BOOT_START_ADDRESS "0x1e000"
    BOOT_MODULE_START_ADDRESS "0x1C000"
  )
else()
  set_target_properties(blinky_boot
    PROPERTIES
    BOOT_START_ADDRESS "0xe000"
    BOOT_MODULE_START_ADDRESS "0xC000"
  )
endif()
make_avr_bootloader(blinky_boot)
