function(make_avr_app target)
    set(app ${target}.app)
    get_target_property(app_module_start ${target} APP_MODULE_START_ADDRESS)
    if(NOT app_module_start)
        set (app_module_start 0x0000)
    endif()
    add_custom_command(
        TARGET ${target}
        POST_BUILD
        BYPRODUCTS ${app}.hex ${app}.bin ${app}.all.hex
        COMMAND avr-size ${target}
        COMMAND cp ${target} ${app}.elf
        COMMAND avr-objcopy -O ihex ${target} ${app}.hex
        COMMAND avr-objcopy -O binary ${target} ${app}.bin
        COMMAND cat "$<TARGET_PROPERTY:${target},LINK_MODULES_BINS>" > ${app}.mods.bin
        COMMAND "$<$<BOOL:$<TARGET_PROPERTY:${target},LINK_MODULES_BINS>>:avr-objcopy;-I;binary;-O;ihex;${app}.mods.bin;${app}.mods.hex;--change-addresses;${app_module_start}>"
        COMMAND srec_cat -O ${app}.all.hex "$<$<BOOL:$<TARGET_PROPERTY:${target},LINK_MODULES_BINS>>:-Intel;${app}.mods.hex>" -Intel ${app}.hex -Intel || { echo "Try increasing APP_MODULE_START_ADDRESS for target ${target}" \\\; exit 1 \\\; }
        COMMAND_EXPAND_LISTS
    )

    add_custom_target(
        ${target}_flash
        DEPENDS ${app}.hex
        COMMAND avrdude -p ${MCU} -c ${PROGRAMMER} -U flash:w:${app}.hex
    )
    add_custom_target(
        ${target}_flash_all
        DEPENDS ${app}.all.hex
        COMMAND avrdude -p ${MCU} -c ${PROGRAMMER} -U flash:w:${app}.all.hex
    )
endfunction()


add_subdirectory(blinky)
add_subdirectory(audio_test)
add_subdirectory(encoder_test)
add_subdirectory(lcd_test)
add_subdirectory(gfx_test)