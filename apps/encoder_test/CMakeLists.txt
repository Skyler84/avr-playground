if(NOT "${BOARD}" STREQUAL "lafortuna")
    return()
endif()

add_executable(encoder_test)
target_sources(encoder_test
    PRIVATE
        main.c
        encoder.c
)

target_link_modules(encoder_test
    PRIVATE
        lcd
)

target_link_libraries(encoder_test
    PRIVATE
        module
)

set_target_properties(encoder_test
    PROPERTIES
        APP_MODULE_START_ADDRESS 0x4000
)

make_avr_app(encoder_test)