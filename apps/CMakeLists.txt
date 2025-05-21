function(make_avr_app target)
    set(app ${target}.app)
    get_target_property(app_module_start ${target} APP_MODULE_START_ADDRESS)
    if(NOT app_module_start)
        set (app_module_start 0x0000)
    endif()

    set(app ${target}.app)
    add_executable(${app})
    add_dependencies(${app} ${target})
    set_target_properties(${app} PROPERTIES
        SOURCES $<TARGET_PROPERTY:${target},SOURCES>
        LINK_MODULES $<TARGET_PROPERTY:${target},LINK_MODULES>
        LINK_MODULES_BINS $<TARGET_PROPERTY:${target},LINK_MODULES_BINS>
        INCLUDE_DIRECTORIES $<TARGET_PROPERTY:${target},INCLUDE_DIRECTORIES>
        LINK_LIBRARIES $<TARGET_PROPERTY:${target},LINK_LIBRARIES>
    )

    add_custom_command(
        TARGET ${app}
        POST_BUILD
        COMMAND avr-size ${target}
    )

    add_custom_target(${app}.hex ALL
        DEPENDS ${app}
        COMMAND avr-objcopy -O ihex ${app} ${app}.hex
    )

    add_custom_target(${app}.bin ALL
        DEPENDS ${app}
        COMMAND avr-objcopy -O binary ${app} ${app}.bin
    )

    get_target_property(mod_bins ${target} LINK_MODULES_TARGETS)
    if ("${mod_bins}" STREQUAL "mod_bins-NOTFOUND")
        set(mod_bins "")
    endif()

    add_custom_target(${app}.mods.bin ALL
        DEPENDS ${mod_bins}
        COMMAND cat "$<TARGET_PROPERTY:${target},LINK_MODULES_BINS>" > ${app}.mods.bin
        COMMAND_EXPAND_LISTS
    )

    add_custom_target(${app}.mods.hex ALL
        DEPENDS ${app}.mods.bin
        COMMAND "$<$<BOOL:$<TARGET_PROPERTY:${target},LINK_MODULES_BINS>>:avr-objcopy;-I;binary;-O;ihex;${app}.mods.bin;${app}.mods.hex;--change-addresses;${app_module_start}>"
        COMMAND_EXPAND_LISTS
    )

    add_custom_target(${app}.all.hex ALL
        DEPENDS ${app}.hex ${app}.mods.hex
        COMMAND srec_cat -O ${app}.all.hex "$<$<BOOL:$<TARGET_PROPERTY:${target},LINK_MODULES_BINS>>:-Intel;${app}.mods.hex>" -Intel ${app}.hex -Intel || { echo "Try increasing APP_MODULE_START_ADDRESS for target ${target}" \\\; exit 1 \\\; }
        COMMAND_EXPAND_LISTS
    )

    add_custom_target(${app}.all.elf ALL
        DEPENDS ${app}.all.hex
        COMMAND avr-objcopy -I ihex -O elf32-avr ${app}.all.hex ${app}.all.elf
    )

    add_custom_target(${app}.all.bin ALL
        DEPENDS ${app}.all.hex
        COMMAND avr-objcopy -I ihex -O binary ${app}.all.hex ${app}.all.bin
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

    install(
        FILES 
        # ${CMAKE_CURRENT_BINARY_DIR}/${app}
        # ${CMAKE_CURRENT_BINARY_DIR}/${app}.hex
        ${CMAKE_CURRENT_BINARY_DIR}/${app}.bin
        # ${CMAKE_CURRENT_BINARY_DIR}/${app}.all.hex
        ${CMAKE_CURRENT_BINARY_DIR}/${app}.all.bin
        DESTINATION ${CMAKE_INSTALL_PREFIX}/apps/${target}
    )

    set_target_properties(
        ${app} ${app}.hex ${app}.bin ${app}.all.hex ${app}.all.elf ${app}.all.bin
        PROPERTIES
        FOLDER "Apps/${target}"
    )

    set_target_properties(
        ${app}.mods.bin ${app}.mods.hex
        PROPERTIES
        FOLDER "Apps/${target}/Modules"
    )

    set_target_properties(${app}_flash ${app}_flash_all PROPERTIES
        FOLDER "Apps/${target}/Flash"
    )

endfunction()


add_subdirectory(blinky)
add_subdirectory(audio_test)
add_subdirectory(encoder_test)
add_subdirectory(lcd_test)
add_subdirectory(gfx_test)
add_subdirectory(fat_test)