#pragma once

#include <stdint.h>

/* Colour definitions RGB565 */
#define WHITE       0xFFFF
#define BLACK       0x0000
#define BLUE        0x001F      
#define GREEN       0x07E0      
#define CYAN        0x07FF      
#define RED         0xF800      
#define MAGENTA     0xF81F      
#define YELLOW      0xFFE0     

typedef uint16_t display_xcoord_t;
typedef uint16_t display_ycoord_t;
typedef uint16_t display_colour_t;