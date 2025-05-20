function(make_avr_bootloader target)
    get_target_property(boot_start ${target} BOOT_START_ADDRESS)
    if(NOT boot_start)
        message(FATAL_ERROR "BOOT_START_ADDRESS property not set for target ${target}")
    endif()
    get_target_property(boot_module_start ${target} BOOT_MODULE_START_ADDRESS)
    if(NOT boot_module_start)
        message(FATAL_ERROR "BOOT_MODULE_START_ADDRESS property not set for target ${target}")
    endif()

    set(app ${target}.boot)
    add_executable(${app})
    add_dependencies(${app} ${target})
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
        # TARGET ${app}
        # POST_BUILD
        # BYPRODUCTS ${app}.hex ${app}.bin ${app}.all.hex ${app}.gdbinit
        OUTPUT ${app}.hex ${app}.bin ${app}.all.hex
    DEPENDS ${app} $<TARGET_PROPERTY:${target},LINK_MODULES_BINS>
        COMMAND avr-size ${app}
        COMMAND cp ${app} ${app}.elf
        COMMAND avr-objcopy -O ihex ${app} ${app}.hex
        COMMAND avr-objcopy -O binary ${app} ${app}.bin
        COMMAND cat "$<TARGET_PROPERTY:${target},LINK_MODULES_BINS>" > ${app}.mods.bin
        COMMAND "$<$<BOOL:$<TARGET_PROPERTY:${target},LINK_MODULES_BINS>>:avr-objcopy;-I;binary;-O;ihex;${app}.mods.bin;${app}.mods.hex;--change-addresses;${boot_module_start}>"
        COMMAND srec_cat -O ${app}.all.hex "$<$<BOOL:$<TARGET_PROPERTY:${target},LINK_MODULES_BINS>>:-Intel;${app}.mods.hex>" -Intel ${app}.hex -Intel || { echo "Try lowering BOOT_MODULE_START_ADDRESS for target ${target}" \\\; exit 1 \\\; }
        COMMAND avr-objcopy -I ihex -O elf32-avr ${app}.all.hex ${app}.all.elf
        COMMAND avr-objcopy -I ihex -O binary ${app}.all.hex ${app}.all.bin
        COMMAND avr-size ${app}.all.hex
        COMMAND_EXPAND_LISTS
    )

    add_custom_target(
        ${app}_flash
        DEPENDS ${app}.hex
        COMMAND avrdude -p ${MCU} -c ${PROGRAMMER} -U flash:w:${app}.hex
    )
    add_custom_target(
        ${app}_flash_all
        DEPENDS ${app}.all.hex
        COMMAND avrdude -p ${MCU} -c ${PROGRAMMER} -U flash:w:${app}.all.hex
    )
endfunction()

add_subdirectory(sdboot)
