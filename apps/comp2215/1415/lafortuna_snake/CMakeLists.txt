add_executable(lafortuna-snake)
target_sources(lafortuna-snake
    PRIVATE
        La-Fortuna-Snake/led.c
        La-Fortuna-Snake/os.c
        La-Fortuna-Snake/main.c
        La-Fortuna-Snake/wav/audio.c
        La-Fortuna-Snake/fatfs/ff.c
        La-Fortuna-Snake/fatfs/sdmm.c
        La-Fortuna-Snake/lcd/lcd.c
        La-Fortuna-Snake/rios/rios.c
        La-Fortuna-Snake/ruota/ruota.c
)

target_include_directories(lafortuna-snake
    PRIVATE
        La-Fortuna-Snake
        La-Fortuna-Snake/lcd
        La-Fortuna-Snake/rios
        La-Fortuna-Snake/ruota
        La-Fortuna-Snake/wav
        La-Fortuna-Snake/fatfs
        La-Fortuna-Snake/serial
)

set_target_properties(lafortuna-snake PROPERTIES 
  FOLDER "comp2215/1415"
)

make_avr_app(lafortuna-snake)