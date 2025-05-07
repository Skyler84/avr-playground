if(NOT "${BOARD}" STREQUAL "lafortuna")
    return()
endif()

add_executable(game_backgammon)
target_sources(game_backgammon
    PRIVATE
        main.c
)

target_link_modules(game_backgammon
  PRIVATE
)

target_link_libraries(game_backgammon
  PRIVATE
    module
    display
    lcd
    fonts
    gfx
)

set_target_properties(game_backgammon
    PROPERTIES
        APP_MODULE_START_ADDRESS 0x4000
)

make_avr_app(game_backgammon)