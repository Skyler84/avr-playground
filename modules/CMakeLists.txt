function(target_link_modules target)
    set (TARGET ${ARGV0})
    set (MODE "PUBLIC")
    foreach(module ${ARGV})
        if ("${module}" STREQUAL "PRIVATE")
            set (MODE "PRIVATE")
            continue()
        endif()
        if ("${module}" STREQUAL "PUBLIC")
            set (MODE "PUBLIC")
            continue()
        endif()
        if ("${module}" STREQUAL "INTERFACE")
            set (MODE "INTERFACE")
            continue()
        endif()
        if ("${module}" STREQUAL "${target}")
            continue()
        endif()
        if (TARGET "${module}")
            target_link_libraries(${TARGET} ${MODE} "mod_interface_${module}")
            get_target_property(link_modules ${module} LINK_MODULES)
            message(STATUS "Linking target ${TARGET} with module ${module} link_modules: ${link_modules}")
            if ("${link_modules}" STREQUAL "link_modules-NOTFOUND")
                set(link_modules "")
            endif()
            list(APPEND link_modules "${module}")
            message(STATUS "${link_modules}")
            set_property(TARGET ${TARGET} APPEND PROPERTY LINK_MODULES "mod_${module}")
            get_target_property(mod_bin mod_${module} BINFILE)
            get_target_property(target_bin mod_${module} BINTARGETS)
            add_dependencies(${TARGET} mod_${module} mod_${module}.bin)
            set_property(TARGET ${TARGET} APPEND PROPERTY LINK_MODULES_BINS "${mod_bin}")
            set_property(TARGET ${TARGET} APPEND PROPERTY LINK_MODULES_TARGETS "${target_bin}")
        else()
            message (FATAL_ERROR "Module ${module} not found")
        endif()
        # if we're linking a module to another module, link to the mod_interface
        if (TARGET "mod_${TARGET}")
            target_link_libraries(mod_${TARGET} INTERFACE mod_interface_${module})
            target_link_libraries(mod_${TARGET} INTERFACE mod_interface_${module})
        endif()
        
    endforeach()
    
endfunction()

function(add_module name)
    add_library(${name} STATIC)
    target_compile_definitions(${name}
        PUBLIC
            ${name}_MODTYPE=STATIC
        PRIVATE
            MODULE_AS_STATIC_LIB
    )
    add_custom_command(
        TARGET ${name}
        POST_BUILD
        COMMAND avr-size lib${name}.a
    )
    # create both a static library
    # and an interface library with hex files
    add_executable(mod_${name})
    target_compile_definitions(mod_${name}
        PUBLIC
            # ${name}_MODTYPE=MODULE
    )

    add_custom_command(
        TARGET mod_${name}
        POST_BUILD
        COMMAND avr-size mod_${name}
        COMMAND ${CMAKE_SOURCE_DIR}/modules/check_module.sh mod_${name}
    )

    add_custom_target(mod_${name}.hex ALL
        DEPENDS mod_${name}
        COMMAND avr-objcopy -O ihex mod_${name} mod_${name}.hex
    )

    add_custom_target(mod_${name}.bin ALL
        DEPENDS mod_${name}
        COMMAND avr-objcopy -O binary mod_${name} mod_${name}.bin
    )

    set_target_properties(mod_${name} PROPERTIES
        BINFILE "${CMAKE_CURRENT_BINARY_DIR}/mod_${name}.bin"
        BINTARGETS "mod_${name}.bin"
    )
    add_library(mod_interface_${name} INTERFACE)
    target_compile_options(mod_interface_${name}
        INTERFACE
            -U${name}_MODTYPE
            -D${name}_MODTYPE=MODULE
    )
    target_compile_options(mod_${name}
        PRIVATE
            -nostartfiles
            -nostdlib
            -U${name}_MODTYPE
            -D${name}_MODTYPE=MODULE
            -UMODULE_AS_STATIC_LIB
    )
    target_compile_definitions(mod_interface_${name}
        INTERFACE
            ${name}_MODTYPE=MODULE
    )
    target_link_options(mod_${name}
        PRIVATE
            -nostartfiles    
            -T ${CMAKE_SOURCE_DIR}/modules/module/ldscripts/module.x
    )
    target_sources(mod_${name}
        PRIVATE
            "$<TARGET_PROPERTY:${name},SOURCES>"
    )
    target_include_directories(mod_interface_${name}
        INTERFACE
            "$<TARGET_PROPERTY:${name},INCLUDE_DIRECTORIES>"
    )
    target_include_directories(mod_${name}
        PUBLIC
            "$<TARGET_PROPERTY:${name},INCLUDE_DIRECTORIES>"
        PRIVATE
            "$<TARGET_PROPERTY:${name},INCLUDE_DIRECTORIES>"
    )
    target_compile_definitions(mod_${name}
        PRIVATE
            "$<TARGET_PROPERTY:${name},COMPILE_DEFINITIONS>"
    )
    # target_
    target_compile_options(mod_${name}
        PRIVATE
            "$<TARGET_PROPERTY:${name},COMPILE_OPTIONS>"
            -B ${CMAKE_SOURCE_DIR}/modules/module/device-specs
    )
    target_link_options(mod_${name}
        PRIVATE
            "$<TARGET_PROPERTY:${name},LINK_OPTIONS>"
            -B ${CMAKE_SOURCE_DIR}/modules/module/device-specs
    )
    if (NOT "${name}" STREQUAL "module")
        target_link_libraries(${name} PRIVATE mod_interface_module)
    endif()

    set_target_properties(
        ${name} 
        mod_${name} 
        mod_interface_${name}
        mod_${name}.bin
        mod_${name}.hex
        PROPERTIES
        FOLDER "Modules/${name}"
    )

    install(
        FILES 
        # ${CMAKE_CURRENT_BINARY_DIR}/mod_${name}.hex
        ${CMAKE_CURRENT_BINARY_DIR}/mod_${name}.bin
        DESTINATION ${CMAKE_INSTALL_PREFIX}/modules/${name}
    )

endfunction()

add_subdirectory(module)
add_subdirectory(font5x7)
add_subdirectory(lcd)
add_subdirectory(sd)
add_subdirectory(vfs)
add_subdirectory(fat)
add_subdirectory(gfx)
add_subdirectory(boot)
add_subdirectory(buttons)
add_subdirectory(gui)