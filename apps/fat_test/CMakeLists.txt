add_executable(fat_test)
target_sources(fat_test
    PRIVATE
      main.c
)
target_link_libraries(fat_test
    PRIVATE
        fat
        sd
        gfx
        lcd
        font5x7
        module
        # softserial
) 

make_avr_app(fat_test)