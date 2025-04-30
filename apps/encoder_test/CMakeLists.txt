add_executable(encoder_test)
target_sources(encoder_test
    PRIVATE
        main.c
)

avr_add_extra_outputs(encoder_test)