#include "lcd/lcd.h"
#include "gfx/gfx.h"
#include "module/imports.h"

// #include "softserial.h"

#include <avr/io.h>
#include <util/delay.h>

#define BLACK_SPACE_COLOUR (0x0841U*0x00U)
#define WHITE_SPACE_COLOUR (0x0841U*0x18U)
#define BLACK_PIECE_COLOUR BLUE
#define WHITE_PIECE_COLOUR GREEN

#define TRIANGLE_LENGTH 80
#define TRIANGLE_WIDTH 20
#define TRIANGLE_SPACING 21


MODULE_IMPORT_FUNCTIONS(lcd, LCD_MODULE_ID, LCD_FUNCTION_EXPORTS);
MODULE_IMPORT_FUNCTIONS(gfx, GFX_MODULE_ID, GFX_FUNCTION_EXPORTS);

lcd_t lcd;
gfx_t gfx;

typedef struct {
  int8_t spaces[4*6+2];
}backgammon_board_t;

typedef struct {
  backgammon_board_t board;
  uint8_t turn_no;
  uint8_t current_player; // 1 is white-, 2 is black+
  uint8_t movements[4];

} backgammon_game_t;

backgammon_board_t board;

void roll_dice(backgammon_game_t *game) {
  game->movements[0] = rand() % 6 + 1;
  game->movements[1] = rand() % 6 + 1;
  if (game->movements[0] == game->movements[1]) {
    game->movements[2] = rand() % 6 + 1;
    game->movements[3] = rand() % 6 + 1;
  } else {
    game->movements[2] = 0;
    game->movements[3] = 0;
  }
}

bool is_valid_move(backgammon_game_t *game, uint8_t /* from */, uint8_t to) {
  int8_t num_pcs = game->board.spaces[to];
  if (num_pcs > 0 && game->current_player == 1) {
    return false;
  } 
  if (num_pcs < 0 && game->current_player == 2) {
    return false;
  }
  // check we have adeuate dice rolls
}

bool make_move(backgammon_game_t *game, uint8_t from, uint8_t to) {
  if (!is_valid_move(game, from, to)) {
    return false;
  }
  game->board.spaces[to] += game->board.spaces[from];
  game->board.spaces[from] = 0;
  return true;
}

void init_board(backgammon_board_t *board) {
  for (uint8_t i = 0; i < 4*6+2; i++) {
    board->spaces[i] = 0;
  }
  board->spaces[      1] = -2;
  board->spaces[25 -  1] =  2;
  board->spaces[      6] =  5;
  board->spaces[25 -  6] = -5;
  board->spaces[      8] =  3;
  board->spaces[25 -  8] = -3;
  board->spaces[     12] = -5;
  board->spaces[25 - 12] =  5;

}

void draw_board() {
  init_board(&board);
  uint16_t board_top = 3;
  uint16_t board_bottom = 240-3;
  uint16_t board_left = 3;
  uint16_t board_right = 320-3;
  uint8_t board_border = 5;
  {
    gfx_region_t r1 = {
      .x1 = board_top,
      .y1 = board_left,
      .x2 = board_bottom,
      .y2 = (board_left+board_right)/2,
    };
    gfx_region_t r2 = {
      .y1 = (board_left+board_right)/2 + 0,
      .x1 = board_top,
      .y2 = board_right,
      .x2 = board_bottom,
    };
    MODULE_CALL_THIS(gfx, fill, &gfx, RED);
    MODULE_CALL_THIS(gfx, stroke, &gfx, 0x0821);
    MODULE_CALL_THIS(gfx, strokeWeight, &gfx, 1);
    MODULE_CALL_THIS(gfx, rectangle, &gfx, r1);
    MODULE_CALL_THIS(gfx, rectangle, &gfx, r2);
  }
  {
    gfx_region_t r1 = {
      .x1 = board_top + board_border,
      .y1 = board_left + board_border,
      .x2 = board_bottom - board_border,
      .y2 = (board_left+board_right)/2 - board_border,
    };
    gfx_region_t r2 = {
      .y1 = (board_left+board_right)/2 + board_border,
      .x1 = board_top + board_border,
      .y2 = board_right - board_border,
      .x2 = board_bottom - board_border,
    };
    MODULE_CALL_THIS(gfx, fill, &gfx, 0x7800);
    // MODULE_CALL_THIS(gfx, stroke, &gfx, 0x03e0);
    MODULE_CALL_THIS(gfx, nostroke, &gfx);
    MODULE_CALL_THIS(gfx, strokeWeight, &gfx, 3);
    MODULE_CALL_THIS(gfx, rectangle, &gfx, r1);
    MODULE_CALL_THIS(gfx, rectangle, &gfx, r2);
  }
  MODULE_CALL_THIS(gfx, nostroke, &gfx);
  int16_t th = (board_bottom - board_top - 2*board_border) * 2/5;
  int16_t tw = (board_right - board_left - 4*board_border) / 14;
  for (uint8_t i = 0; i < 24; i++) {
    MODULE_CALL_THIS(gfx, fill, &gfx, i%2 ? BLACK_SPACE_COLOUR : WHITE_SPACE_COLOUR);
    gfx_coord_t tps[3] = {{
      .y = tw/2,
      .x = th,
    },{
      .y = 0,
      .x = 0,
    },{
      .y = tw,
      .x = 0,
    }};
    for (uint8_t j = 0; j < 3; j++) {
      tps[j].y += ((i%12)+1)*tw + board_border+1;
      tps[j].x += board_border;
      if (i%12 >= 6) {
        tps[j].y += 10;
      }
      if (i >= 12) {
        tps[j].y = board_left + tps[j].y;
        tps[j].x = board_top + tps[j].x;
      } else {
        tps[j].y = board_right - tps[j].y;
        tps[j].x = board_bottom - tps[j].x;
      }
    }
    MODULE_CALL_THIS(gfx, triangle, &gfx, 
      tps[0], 
      tps[1], 
      tps[2]);
      
    int8_t pcs = board.spaces[i+1];
    gfx_colour_t col = pcs > 0 ? BLACK_PIECE_COLOUR : WHITE_PIECE_COLOUR;
    if (pcs < 0) {
      pcs = -pcs;
    }
    for (uint8_t k = 0; k < pcs; k++) {
      gfx_coord_t c = {
        .x = tps[1].x,
        .y = tps[0].y,
      };
      if (i >= 12) {
        c.x += (k*2+1)*tw/2;
      } else {
        c.x -= (k*2+1)*tw/2;
      }
      MODULE_CALL_THIS(gfx, fill, &gfx, col);
      MODULE_CALL_THIS(gfx, circle, &gfx, c, TRIANGLE_WIDTH/2);
    }
  }
  for (uint8_t i = 0; i < 24; i+=24-1) {
    
    // int8_t pcs = board.spaces[i+1];
    // gfx_colour_t col = pcs > 0 ? BLACK_PIECE_COLOUR : WHITE_PIECE_COLOUR;
    // if (pcs < 0) {
    //   pcs = -pcs;
    // }
    // for (uint8_t k = 0; k < pcs; k++) {
    //   gfx_coord_t r = {
    //     .x = tps[1].x,
    //     .y = tps[0].y,
    //   };
    //   if (i < 12) {
    //     c.x += (k*2+1)*TRIANGLE_WIDTH/2;
    //   } else {
    //     c.x -= (k*2+1)*TRIANGLE_WIDTH/2;
    //   }
    //   MODULE_CALL_THIS(gfx, fill, &gfx, col);
    //   MODULE_CALL_THIS(gfx, rectangle, &gfx, c, TRIANGLE_WIDTH/2);
    // }
  }

}

int main() {
  
  // set clock prescaler to 1
  CLKPR = _BV(CLKPCE); // Enable change
  CLKPR = 0; // Set prescaler to 1

  lcd.fns = &lcd_fns;
  gfx.fns = &gfx_fns;

  MODULE_CALL_THIS(display, init, &lcd.display);
  MODULE_CALL_THIS(lcd, set_orientation, &lcd, West);
  MODULE_CALL_THIS(gfx, init, &gfx, &lcd.display);
  MODULE_CALL_THIS(gfx, fill, &gfx, RED);
  MODULE_CALL_THIS(gfx, nostroke, &gfx);

  draw_board();
  DDRB = 0x80;
  while(1) {
    // PINB = 0x80;
    // _delay_ms(1000);
  }
}