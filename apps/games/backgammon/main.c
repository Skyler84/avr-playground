#include "lcd/lcd.h"
#include "gfx/gfx.h"
#include "module/imports.h"

// #include "softserial.h"

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#define BLACK_SPACE_COLOUR (0x0841U * 0x00U)
#define WHITE_SPACE_COLOUR (0x0841U * 0x18U)
#define BLACK_PIECE_COLOUR BLUE
#define WHITE_PIECE_COLOUR GREEN

#define WHITE_HOME 0
#define BLACK_HOME 25

#define TRIANGLE_LENGTH 80
#define TRIANGLE_WIDTH 20
#define TRIANGLE_SPACING 21

#define PIECE_RADIUS 10
#define PIECE_STROKEWIDTH 2

struct backgammon_colour_theme
{
  gfx_colour_t board_col;
  gfx_colour_t black_space_col;
  gfx_colour_t white_space_col;
  gfx_colour_t black_piece_col;
  gfx_colour_t white_piece_col;
};

/**
 *
 * Move syntax:
 * "4-4:" represents rolling a double 4
 * "3-1:" represents rolling a 3 and a 1
 * "8/4" represents moving a piece from 8 to 4
 * "4-2:8/4 6/4" represents moving a piece from 8 to 4 and then from 6 to 4 having rolled a 2 and a 4
 * "22/off" represents moving a piece from 22 to home
 * "bar/4" represents moving a piece from the bar to 4
 * "6/4(3)" represents moving 3 pieces from 6 to 4
 * "13/7*\/5" represents moving a piece from 13 to 7 and then from 7 to 5, hitting a piece on the way
 */

char move_buf[32];

MODULE_IMPORT_FUNCTIONS(lcd, LCD_MODULE_ID, LCD_FUNCTION_EXPORTS);
MODULE_IMPORT_FUNCTIONS(gfx, GFX_MODULE_ID, GFX_FUNCTION_EXPORTS);

lcd_t lcd;
gfx_t gfx;

typedef struct
{
  int8_t spaces[4 * 6 + 2];
  int8_t bar[2]; // 0 is white, 1 is black
} backgammon_board_t;

typedef struct
{
  backgammon_board_t board;
  uint8_t turn_no;
  uint8_t current_player; // 1 is white-, 2 is black+
  uint8_t movements[4];

} backgammon_game_t;

backgammon_game_t game;

void roll_dice(backgammon_game_t *game);
bool make_move(backgammon_game_t *game, const char *move, bool dry_run);
bool moves_available(backgammon_game_t *game);
void init_board(backgammon_board_t *board);
void draw_board(backgammon_board_t *board);

void roll_dice(backgammon_game_t *game)
{
  game->movements[0] = rand() % 6 + 1;
  game->movements[1] = rand() % 6 + 1;
  if (game->movements[0] == game->movements[1])
  {
    game->movements[2] = rand() % 6 + 1;
    game->movements[3] = rand() % 6 + 1;
  }
  else
  {
    game->movements[2] = 0;
    game->movements[3] = 0;
  }
}

uint8_t parseint(const char **str)
{
  uint8_t result = 0;
  while (**str >= '0' && **str <= '9')
  {
    result = result * 10 + (**str - '0');
    (*str)++;
  }
  return result;
}
uint8_t advance_to(const char **str, char c)
{
  while (**str != c && **str != '\0')
  {
    (*str)++;
  }
  if (**str == c)
  {
    (*str)++;
  }
  return **str == '\0';
}

void consume_prefix(const char **str, const char *prefix)
{
  while (*prefix != '\0' && **str == *prefix)
  {
    (*str)++;
    prefix++;
  }
  if (*prefix != '\0')
  {
    // didn't match the prefix
    *str = NULL;
  }
}

void undo_move(backgammon_game_t *game, const char *move)
{
  // undo the move
  const char *m = move;
  uint8_t from, to;
  int8_t *fromptr = NULL, *toptr = NULL;
  if (strncmp(m, "bar", 3) == 0)
  {
    m += 3; // bar
    fromptr = &game->board.bar[game->current_player - 1];
    if (game->current_player == 1)
    {
      from = BLACK_HOME;
    }
    else if (game->current_player == 2)
    {
      from = WHITE_HOME;
    }
  }
  else
  {
    from = parseint(&m);
    if (from > 24 || from < 1)
    {
      return; // invalid from space
    }
    fromptr = &game->board.spaces[from];
  }
  if (*m != '/')
  {
    return;
  }
  m++;

  if (strncmp(m, "off", 3) == 0)
  {
    m += 3; // off
    if (game->current_player == 1)
    {
      to = WHITE_HOME;
    }
    else if (game->current_player == 2)
    {
      to = BLACK_HOME;
    }
  }
  else
  {
    to = parseint(&m);
  }
  toptr = &game->board.spaces[to];
}

bool make_move(backgammon_game_t *game, const char *_move, bool dry_run)
{
  // check some basic conditions

  if (game->current_player != 1 && game->current_player != 2)
  {
    return false; // invalid player
  }

  const char *move = _move;
  uint8_t from, to;
  int8_t *fromptr = NULL, *toptr = NULL;
  int8_t *barptr = &game->board.bar[game->current_player - 1];
  uint8_t num_pcs_to_move = 1;
  {
    // decode move
    if (strncmp(move, "bar", 3) == 0)
    {
      move += 3; // bar
      fromptr = &game->board.bar[game->current_player - 1];
      if (game->current_player == 1)
      {
        from = BLACK_HOME;
      }
      else if (game->current_player == 2)
      {
        from = WHITE_HOME;
      }
    }
    else
    {
      from = parseint(&move);
      if (from > 24 || from < 1)
      {
        return false; // invalid from space
      }
      fromptr = &game->board.spaces[from];
    }
    if (*move != '/')
    {
      return false;
    }
    move++;

    if (strncmp(move, "off", 3) == 0)
    {
      move += 3; // off
      if (game->current_player == 1)
      {
        to = WHITE_HOME;
      }
      else if (game->current_player == 2)
      {
        to = BLACK_HOME;
      }
    }
    else
    {
      to = parseint(&move);
    }
    toptr = &game->board.spaces[to];

    if (*move != '\0' && *move != ' ' && *move != '*' && *move != '/')
    {
      return false;
    }
    bool take = *move == '*';
    if (take)
    {
      move++;
    }
    if (*move == '(')
    {
      move++;
      num_pcs_to_move = parseint(&move);
      if (*move != ')')
      {
        return false; // invalid move
      }
      move++;
    }
  }
  int8_t num_pcs_from = game->board.spaces[from];
  int8_t num_pcs_to = game->board.spaces[to];
  int8_t move_dist = to - from;
  int8_t sign = -1; // BLACK is +, WHITE is - 
  if (game->current_player == 2 - (sign+1)/2)
  {
    num_pcs_from = -num_pcs_from;
    num_pcs_to = -num_pcs_to;
    move_dist = -move_dist;
    sign = -sign;
  }
  if (num_pcs_from > -num_pcs_to_move)
  {
    return false; // can't move opponent's pieces
  }
  if (num_pcs_to > 1)
  {
    return false; // can't move to a space with more than 1 opponent's piece
  }
  if (take == (num_pcs_to != 1))
  {
    return false; // can't take a piece if there isn't one there and must take if there is one there
  }
  if (move_dist < 1 || move_dist > 6)
  {
    return false; // invalid move distance
  }
  uint8_t move_no = 255;
  uint8_t i;
  for (i = 0; i < 4 && game->movements[i] > 0; i++)
  {
    if (game->movements[i] == move_dist)
    {
      move_no = i;
    }
  }
  i--;
  if (move_no == 255)
  {
    return false; // invalid move distance
  }
  if (take)
  {
    *toptr += sign;
    game->board.bar[game->current_player - 1]++;
  }
  *fromptr -= sign*num_pcs_to_move;
  *toptr += sign*num_pcs_to_move;
  {
    // swap and invert the move
    uint8_t tmp = game->movements[move_no];
    game->movements[move_no] = game->movements[i];
    game->movements[i] = -tmp;
  }
  if (dry_run)
  {
    // undo all that again
    *fromptr += sign*num_pcs_to_move;
    *toptr -= sign*num_pcs_to_move;
    if (take)
    {
      *toptr -= sign;
      game->board.bar[game->current_player - 1]--;
    }
    {
      uint8_t tmp = game->movements[i];
      game->movements[i] = game->movements[move_no];
      game->movements[move_no] = -tmp;
    }
  }
  return true;
}

void init_board(backgammon_board_t *board)
{
  for (uint8_t i = 0; i < 4 * 6 + 2; i++)
  {
    board->spaces[i] = 0;
  }
  board->spaces[1] = -2;
  board->spaces[25 - 1] = 2;
  board->spaces[6] = 5;
  board->spaces[25 - 6] = -5;
  board->spaces[8] = 3;
  board->spaces[25 - 8] = -3;
  board->spaces[12] = -5;
  board->spaces[25 - 12] = 5;
}

void draw_board(backgammon_board_t *board)
{
  uint16_t board_top = 3;
  uint16_t board_bottom = 240 - 3;
  uint16_t board_left = 3;
  uint16_t board_right = 320 - 3;
  uint8_t board_border = 5;
  {
    gfx_region_t r1 = {
        .x1 = board_top,
        .y1 = board_left,
        .x2 = board_bottom,
        .y2 = (board_left + board_right) / 2,
    };
    gfx_region_t r2 = {
        .y1 = (board_left + board_right) / 2 + 0,
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
        .y2 = (board_left + board_right) / 2 - board_border,
    };
    gfx_region_t r2 = {
        .y1 = (board_left + board_right) / 2 + board_border,
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
  int16_t th = (board_bottom - board_top - 2 * board_border) * 2 / 5;
  int16_t tw = (board_right - board_left - 4 * board_border) / 14;
  for (uint8_t i = 0; i < 24; i++)
  {
    MODULE_CALL_THIS(gfx, fill, &gfx, i % 2 ? BLACK_SPACE_COLOUR : WHITE_SPACE_COLOUR);
    gfx_coord_t tps[3] = {{
                              .y = tw / 2,
                              .x = th,
                          },
                          {
                              .y = 0,
                              .x = 0,
                          },
                          {
                              .y = tw,
                              .x = 0,
                          }};
    for (uint8_t j = 0; j < 3; j++)
    {
      tps[j].y += ((i % 12) + 1) * tw + board_border + 1;
      tps[j].x += board_border;
      if (i % 12 >= 6)
      {
        tps[j].y += 10;
      }
      if (i >= 12)
      {
        tps[j].y = board_left + tps[j].y;
        tps[j].x = board_top + tps[j].x;
      }
      else
      {
        tps[j].y = board_right - tps[j].y;
        tps[j].x = board_bottom - tps[j].x;
      }
    }
    MODULE_CALL_THIS(gfx, triangle, &gfx,
                     tps[0],
                     tps[1],
                     tps[2]);

    int8_t pcs = board->spaces[i + 1];
    gfx_colour_t col = pcs > 0 ? BLACK_PIECE_COLOUR : WHITE_PIECE_COLOUR;
    if (pcs < 0)
    {
      pcs = -pcs;
    }
    for (uint8_t k = 0; k < pcs; k++)
    {
      gfx_coord_t c = {
          .x = tps[1].x,
          .y = tps[0].y,
      };
      if (i >= 12)
      {
        c.x += (k * 2 + 1) * tw / 2;
      }
      else
      {
        c.x -= (k * 2 + 1) * tw / 2;
      }
      MODULE_CALL_THIS(gfx, fill, &gfx, col);
      MODULE_CALL_THIS(gfx, circle, &gfx, c, TRIANGLE_WIDTH / 2);
    }
  }
  for (uint8_t i = 0; i < 24; i += 24 - 1)
  {

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

int main()
{

  // set clock prescaler to 1
  CLKPR = _BV(CLKPCE); // Enable change
  CLKPR = 0;           // Set prescaler to 1

  lcd.fns = &lcd_fns;
  gfx.fns = &gfx_fns;

  MODULE_CALL_THIS(display, init, &lcd.display);
  MODULE_CALL_THIS(lcd, set_orientation, &lcd, West);
  MODULE_CALL_THIS(gfx, init, &gfx, &lcd.display);
  MODULE_CALL_THIS(gfx, fill, &gfx, RED);
  MODULE_CALL_THIS(gfx, nostroke, &gfx);

  draw_board(&game.board);
  DDRB = 0x80;
  while (1)
  {
    // PINB = 0x80;
    // _delay_ms(1000);
  }
}