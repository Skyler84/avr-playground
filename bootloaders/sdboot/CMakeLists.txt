if(NOT "${BOARD}" STREQUAL "lafortuna")
    return()
endif()

add_executable(sdboot)
target_sources(sdboot
    PRIVATE
        main.c
        encoder.c
        buttons.c
        gui.c
)

target_link_modules(sdboot
    PRIVATE
        sd
        fat
        lcd
        gfx
        font5x7
)
target_link_libraries(sdboot
    PRIVATE
        ld_debug
        module
        display
        blockdev
        font
        # sd
)

target_compile_options(sdboot
    PRIVATE
        -Os
)

set_target_properties(sdboot PROPERTIES 
    BOOT_START_ADDRESS 0x1E000
    BOOT_MODULE_START_ADDRESS 0x1A000
    APP_MODULE_START_ADDRESS 0x2000
)


make_avr_app(sdboot)
make_avr_bootloader(sdboot)
