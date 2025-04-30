add_module(lcd)
target_sources(lcd
    PRIVATE
        lcd.c
)
target_include_directories(lcd
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_modules(lcd
    PRIVATE
        module
        fonts
)