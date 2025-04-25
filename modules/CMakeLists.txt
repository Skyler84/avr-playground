function(target_link_modules target)
    set (TARGET ${ARGV0})
    foreach(module ${ARGV})
    if (module STREQUAL "PRIVATE")
    continue()
    endif()
    if (module STREQUAL "PUBLIC")
    continue()
    endif()
    if (module STREQUAL "INTERFACE")
    continue()
    endif()
    if (module STREQUAL "${target}")
    continue()
    endif()
        message (STATUS "Linking target ${TARGET} with module ${module}")
        if (TARGET ${module})
            message (STATUS "Linking target ${TARGET} with module ${module}")
            target_link_libraries(${TARGET} PRIVATE mod_interface_${module})
        else()
            message (FATAL_ERROR "Module ${module} not found")
        endif()
        
    endforeach()
    
endfunction()

function(add_module name)
    add_library(${name} STATIC)
    # create both a static library
    # and an interface library with hex files
    add_executable(mod_${name})
    add_library(mod_interface_${name} INTERFACE)
    target_compile_options(mod_${name}
        PRIVATE
            -nostartfiles
            -nostdlib
    )
    target_link_options(mod_${name}
        PRIVATE
            -nostartfiles    
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
    target_compile_options(mod_${name}
        PRIVATE
            "$<TARGET_PROPERTY:${name},COMPILE_OPTIONS>"
    )
    # target_link_libraries(mod_${name}
    #     PRIVATE
    #         "$<TARGET_PROPERTY:${name},LINK_LIBRARIES>"
    # )
    target_link_modules(mod_${name} module)
endfunction()

add_subdirectory(module)
add_subdirectory(fonts)
# add_subdirectory(blockdev)
add_subdirectory(lcd)
add_subdirectory(sd)
add_subdirectory(fat)