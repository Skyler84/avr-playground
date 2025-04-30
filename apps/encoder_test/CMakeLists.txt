if(NOT "${BOARD}" STREQUAL "lafortuna")
    return()
endif()

add_executable(encoder_test)
target_sources(encoder_test
    PRIVATE
        main.c
)

make_avr_app(encoder_test)