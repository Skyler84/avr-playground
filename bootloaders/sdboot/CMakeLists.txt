if(NOT "${BOARD}" STREQUAL "lafortuna")
    return()
endif()

add_subdirectory(interactive)
add_subdirectory(stub)