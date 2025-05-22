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

make_avr_app(TronFortuna)