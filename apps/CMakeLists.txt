function(make_avr_app target)
    set(app ${target}.app)
    add_custom_command(
        TARGET ${target}
        POST_BUILD
        OUTPUTS ${app}.hex ${app}.bin
        COMMAND avr-size ${target}
        COMMAND cp ${target} ${app}.elf
        COMMAND avr-objcopy -O ihex ${target} ${app}.hex
        COMMAND avr-objcopy -O binary ${target} ${app}.bin
        COMMAND cat "$<TARGET_PROPERTY:${target},LINK_MODULES_BINS>" > ${app}.mods.bin
        COMMAND cat ${app}.bin ${app}.mods.bin > ${app}.all.bin
        COMMAND srec_cat -O ${app}.all.hex -Intel ${app}.all.bin -Binary
        COMMAND_EXPAND_LISTS
        VERBATIM
    )
endfunction()


add_subdirectory(audio_test)
add_subdirectory(encoder_test)