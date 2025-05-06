if(NOT "${BOARD}" STREQUAL "lafortuna")
    return()
endif()

add_executable(gfx_test)
target_sources(gfx_test
    PRIVATE
        main.c
)

target_link_modules(gfx_test
  PRIVATE
)

target_link_libraries(gfx_test
  PRIVATE
    module
    display
    lcd
    gfx
)

set_target_properties(gfx_test
    PROPERTIES
        APP_MODULE_START_ADDRESS 0x4000
)

make_avr_app(gfx_test)