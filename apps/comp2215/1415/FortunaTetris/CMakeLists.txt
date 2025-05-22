add_executable(fortuna-tetris)
target_sources(fortuna-tetris
    PRIVATE
        FortunaTetris/switches
        FortunaTetris/tetris
        FortunaTetris/tetromino.c
        FortunaTetris/lcd/lcd.c
)

target_include_directories(fortuna-tetris
    PRIVATE
        FortunaTetris/
        FortunaTetris/lcd/
)
make_avr_app(fortuna-tetris)