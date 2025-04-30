

function(avr_add_extra_outputs target)
    add_custom_command(
        TARGET ${target}
        POST_BUILD
        OUTPUTS ${target}.hex ${target}.bin
        COMMAND avr-size ${target}
        COMMAND avr-objcopy -O ihex -R .eeprom ${target} ${target}.hex
        COMMAND avr-objcopy -O binary -R .eeprom ${target} ${target}.bin
        COMMAND cat "$<TARGET_PROPERTY:${target},LINK_MODULES_BINS>" > ${target}.mods.bin
        COMMAND cat ${target}.bin ${target}.mods.bin > ${target}.all.bin
        COMMAND srec_cat -O ${target}.all.hex -Intel ${target}.all.bin -Binary
        COMMAND_EXPAND_LISTS
        VERBATIM
    )
    
endfunction()


add_subdirectory(audio_test)
add_subdirectory(encoder_test)