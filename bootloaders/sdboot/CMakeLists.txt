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

avr_add_extra_outputs(sdboot)