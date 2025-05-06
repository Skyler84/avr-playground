if(NOT "${BOARD}" STREQUAL "lafortuna")
    return()
endif()

add_executable(lcd_test)
target_sources(lcd_test
    PRIVATE
        main.c
)

target_link_modules(lcd_test
  PRIVATE
    lcd
) 
  
target_link_libraries(lcd_test
  PRIVATE
    module
    display
    fonts
)

set_target_properties(lcd_test
    PROPERTIES
        APP_MODULE_START_ADDRESS 0x4000
)

make_avr_app(lcd_test)