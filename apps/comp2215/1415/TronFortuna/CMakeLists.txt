add_executable(TronFortuna)
target_sources(TronFortuna
    PRIVATE
        TronFortuna/switches.c
        TronFortuna/tron.c
        TronFortuna/lcd/lcd.c
)

target_include_directories(TronFortuna
    PRIVATE
        TronFortuna/
        TronFortuna/lcd/
)

set_target_properties(TronFortuna PROPERTIES 
  FOLDER "comp2215/1415"
)

make_avr_app(TronFortuna)