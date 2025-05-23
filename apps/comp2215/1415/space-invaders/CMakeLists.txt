add_executable(space-invaders)
target_sources(space-invaders
    PRIVATE
        AVR-Space_Invaders/breaker.c
        AVR-Space_Invaders/keyboard.c
        AVR-Space_Invaders/lcd/lcd.c
        AVR-Space_Invaders/encoder/encoder.c
)

target_include_directories(space-invaders
    PRIVATE
        AVR-Space_Invaders
        AVR-Space_Invaders/lcd
        AVR-Space_Invaders/encoder
)

set_target_properties(space-invaders PROPERTIES 
  FOLDER "comp2215/1415"
)

make_avr_app(space-invaders)