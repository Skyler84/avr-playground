

function(avr_add_extra_outputs target)
    add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND avr-size --format=avr --mcu=${MCU} ${target}
        COMMAND avr-objcopy -O ihex -R .eeprom ${target} ${target}.hex
        COMMAND avr-objcopy -O binary -R .eeprom ${target} ${target}.bin
    )
endfunction()


add_subdirectory(audio_test)
add_subdirectory(encoder_test)