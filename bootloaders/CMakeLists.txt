function(make_avr_bootloader target)
    get_target_property(boot_start ${target} BOOT_START_ADDRESS)
    if(NOT boot_start)
        message(FATAL_ERROR "BOOT_START_ADDRESS property not set for target ${target}")
    endif()

    set(app ${target}.boot)
    add_executable(${app})
    set_target_properties(${app} PROPERTIES
        SOURCES $<TARGET_PROPERTY:${target},SOURCES>
        LINK_MODULES $<TARGET_PROPERTY:${target},LINK_MODULES>
        LINK_MODULES_BINS $<TARGET_PROPERTY:${target},LINK_MODULES_BINS>
        INCLUDE_DIRECTORIES $<TARGET_PROPERTY:${target},INCLUDE_DIRECTORIES>
        LINK_LIBRARIES $<TARGET_PROPERTY:${target},LINK_LIBRARIES>
    )
    target_link_options(${app} PRIVATE
        -Wl,--section-start=.text=${boot_start}
        -Wl,--section-start=.bootloader=${boot_start}
        -Wl,--section-start=.bootloader_data=${boot_start}
    )

    add_custom_command(
        TARGET ${app}
        POST_BUILD
        OUTPUTS ${app}.hex ${app}.bin
        COMMAND avr-size ${app}
        COMMAND cp ${app} ${app}.elf
        COMMAND avr-objcopy -O ihex ${app} ${app}.hex
        COMMAND avr-objcopy -O binary ${app} ${app}.bin
        COMMAND cat "$<TARGET_PROPERTY:${target},LINK_MODULES_BINS>" > ${app}.mods.bin
        COMMAND cat ${app}.bin ${app}.mods.bin > ${app}.all.bin
        COMMAND srec_cat -O ${app}.all.hex -Intel ${app}.all.bin -Binary
        COMMAND_EXPAND_LISTS
        VERBATIM
    )
endfunction()

add_subdirectory(sdboot)
