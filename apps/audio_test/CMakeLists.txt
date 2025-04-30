add_executable(audio_test)
target_sources(audio_test
    PRIVATE
        main.c
)

make_avr_app(audio_test)