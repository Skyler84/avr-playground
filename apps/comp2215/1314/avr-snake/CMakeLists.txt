add_executable(avr-snake)
target_sources(avr-snake
    PRIVATE
        avr-snake/src/RIOS.c
        avr-snake/src/avrSnake.c
        avr-snake/lcdlib/font.c
        avr-snake/lcdlib/ili934x.c
        avr-snake/lcdlib/lcd.c
)
target_include_directories(avr-snake
    PRIVATE
        avr-snake/src
        avr-snake/lcdlib
)

make_avr_app(avr-snake)