add_executable(sdboot)
target_sources(sdboot
    PRIVATE
        main.c
        encoder.c
)

target_link_modules(sdboot
    PRIVATE
        fonts
        lcd
        sd
        fat
)
target_link_libraries(sdboot
    PRIVATE
        module
)

set_target_properties(sdboot PROPERTIES 
    BOOT_START_ADDRESS 0xe000
)

make_avr_app(sdboot)
make_avr_bootloader(sdboot)