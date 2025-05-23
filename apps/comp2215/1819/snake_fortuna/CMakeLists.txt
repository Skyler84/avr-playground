add_executable(snake-fortuna)

set(PNGS 
    levels.png
    segment24x32.png
    snake.png
    sqr8x8.png
)

function(png_to_blobs infile outfile)

  string(REPLACE "/" "_" symname ${outfile})
  string(REPLACE "-" "_" symname ${symname})
  # get last part of outfile after last '/'
  string(REGEX REPLACE ".*/" "" newsymname ${outfile})
  message(STATUS "symname: ${symname}")

  foreach(depth 1 2 4)
    add_custom_command(
      OUTPUT ${outfile}_${depth}
      COMMAND convert ${infile} -depth ${depth} -colorspace gray Y:${outfile}_${depth}
      DEPENDS ${infile}
    )
    add_custom_command(
      OUTPUT ${outfile}_${depth}.o
      COMMAND avr-objcopy -B avr51 --rename-section .data=.progmem.data,contents,alloc,load,readonly,data --redefine-sym _binary_${symname}_${depth}_start=${newsymname}_${depth} --redefine-sym _binary_${symname}_${depth}_end=${newsymname}_${depth}_end --redefine-sym _binary_${symname}_${depth}_size=${newsymname}_${depth}_size -I binary -O elf32-avr ${outfile}_${depth} ${outfile}_${depth}.o
      DEPENDS ${outfile}_${depth}
    )

  endforeach()

  
  # add_custom_command(
  #   OUTPUT ${outfile}_2
  #   COMMAND convert ${infile} -depth 2 -colorspace gray Y:${outfile}_2
  #   DEPENDS ${infile} ${outfile}_1.o
  # )
  # add_custom_command(
  #   OUTPUT ${outfile}_2.o
  #   COMMAND ${OBJCOPY} --rename-section .data=.progmem.data,contents,alloc,load,readonly,data -I binary -O elf32-avr ${outfile}_2 ${outfile}_2.o
  #   DEPENDS ${outfile}_2
  # )
  
  # add_custom_command(
  #   OUTPUT ${outfile}_4
  #   COMMAND convert ${infile} -depth 4 -colorspace gray Y:${outfile}_4
  #   DEPENDS ${infile} ${outfile}_2.o
  # )
  # add_custom_command(
  #   OUTPUT ${outfile}_4.o
  #   COMMAND ${OBJCOPY} --rename-section .data=.progmem.data,contents,alloc,load,readonly,data -I binary -O elf32-avr ${outfile}_4 ${outfile}_4.o
  #   DEPENDS ${outfile}_4
  # )

endfunction()
set(png_blobs "")

foreach(png ${PNGS})
    string(REPLACE ".png" "" png_name ${png})
    # string(TOUPPER ${png_name} png_name)
    set(png_out ${CMAKE_CURRENT_BINARY_DIR}/${png_name})
    png_to_blobs(${CMAKE_CURRENT_SOURCE_DIR}/snake-fortuna/img/${png} ${png_out})
    list(APPEND png_blobs 
      ${png_out}_1.o 
      ${png_name}_2.o 
      ${png_name}_4.o
    )
endforeach()

target_sources(snake-fortuna
    PRIVATE
        snake-fortuna/src/blit.c
        snake-fortuna/src/colormul.c
        snake-fortuna/src/rng.c
        snake-fortuna/src/hue.s
        snake-fortuna/src/snake/anybutton.c
        snake-fortuna/src/snake/board.c
        snake-fortuna/src/snake/credits.c
        snake-fortuna/src/snake/entername.c
        snake-fortuna/src/snake/gameloop.c
        snake-fortuna/src/snake/grid.c
        snake-fortuna/src/snake/hiscores.c
        snake-fortuna/src/snake/levels.c
        snake-fortuna/src/snake/main.c
        snake-fortuna/src/snake/mainmenu.c
        snake-fortuna/src/snake/markers.c
        snake-fortuna/src/snake/scoreio.c
        snake-fortuna/src/snake/turns.c
        snake-fortuna/src/hw/buttons.c
        snake-fortuna/src/hw/display.c
        snake-fortuna/src/hw/timer.c
        snake-fortuna/src/font/font.c
        ${png_blobs}
)

target_compile_options(snake-fortuna PRIVATE $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>)

target_include_directories(snake-fortuna
    PRIVATE
        snake-fortuna/src
        snake-fortuna/src/snake
        snake-fortuna/src/hw
        snake-fortuna/src/font
)

target_link_libraries(snake-fortuna
  PRIVATE
    # ${png_blobs}
)

set_target_properties(snake-fortuna PROPERTIES 
  FOLDER "comp2215/1819"
)

make_avr_app(snake-fortuna)