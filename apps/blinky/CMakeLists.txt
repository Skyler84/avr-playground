add_executable(blinky)
target_sources(blinky
    PRIVATE
        main.c
)

make_avr_app(blinky)