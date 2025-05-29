add_executable(sdboot_stub)
target_sources(sdboot_stub
    PRIVATE
        main.c
)

target_link_modules(sdboot_stub
    PRIVATE
        lcd
        gfx
        font5x7
        buttons
        gui
        sdboot_interactive
)

target_link_libraries(sdboot_stub
    PRIVATE
        module
        boot
        sd
        fat
        ld_debug
)


set_target_properties(sdboot_stub PROPERTIES 
    BOOT_START_ADDRESS 0x1E000
    BOOT_MODULE_START_ADDRESS 0x1A000
    APP_MODULE_START_ADDRESS 0x4000
)

make_avr_bootloader(sdboot_stub)