#include "gfx/gfx.h"
#include "module/imports.h"
#include "module/pic.h"
#include <avr/pgmspace.h>
#include <alloca.h>

uint32_t multiply(uint16_t a, uint16_t b)
{
  uint32_t out;
  asm volatile(
      "movw r26, %1\n"
      "movw r18, %2\n"
      "icall\n"
      "movw %A0, r22\n"
      "movw %C0, r24\n"
      : "=r" (out)
      : "r" (a), "r" (b), "z" (indirect_call(__umulhisi3))
      : "r26", "r27", "r18", "r19", "r22", "r23", "r24", "r25"
  );
  return out;
}


#define divide __udivmodhi4
unsigned short __udivmodhi4(unsigned short num, unsigned short den);

void gfx_init(gfx_t *gfx, display_t *display)
{
    gfx->display = display;
    gfx->fill = true;
    gfx->stroke = true;
    gfx->strokeWeight = 1;
    gfx->strokeColour = BLACK;
    gfx->fillColour = WHITE;
    MODULE_CALL_THIS(display, init, gfx->display);
}

gfx_colour_t gfx_colour(uint8_t r, uint8_t g, uint8_t b)
{
    return (gfx_colour_t)(((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3));
}

gfx_colour_t gfx_colourA(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
    return (gfx_colour_t)(((a & 0xf8) << 8) | ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3));
}

void gfx_fill(gfx_t *gfx, gfx_colour_t colour)
{
    gfx->fill = true;
    gfx->fillColour = colour;
}

void gfx_nofill(gfx_t *gfx)
{
    gfx->fill = false;
}
void gfx_stroke(gfx_t *gfx, gfx_colour_t colour)
{
    gfx->stroke = true;
    gfx->strokeColour = colour;
}
void gfx_strokeWeight(gfx_t *gfx, uint8_t weight)
{
    gfx->strokeWeight = weight;
}
void gfx_nostroke(gfx_t *gfx)
{
    gfx->stroke = false;
}
void gfx_point(gfx_t *gfx, gfx_coord_t coord)
{
    if (!gfx->stroke)
    {
        return;
    }
    gfx_region_t r = {
        .x1 = coord.x - gfx->strokeWeight / 2,
        .y1 = coord.y - gfx->strokeWeight / 2,
        .x2 = coord.x + (gfx->strokeWeight - 1) / 2,
        .y2 = coord.y + (gfx->strokeWeight - 1) / 2,
    };
    MODULE_CALL_THIS(display, region_set, gfx->display, r);
    MODULE_CALL_THIS(display, fill, gfx->display, gfx->strokeColour, indirect_call(multiply)(gfx->strokeWeight, gfx->strokeWeight));
}

// static void gfx_fast_hline(gfx_t *gfx, gfx_coord_t start, gfx_coord_t end)
// {
//     if (start.y != end.y) {
//         return;
//     }
//     if (!gfx->stroke) {
//         return;
//     }
//     gfx->display->fns->region_set(gfx->display, (gfx_region_t){start.x, start.y, end.x, end.y});
//     gfx->display->fns->fill(gfx->display, gfx->strokeColour, end.x - start.x + 1);
// }

// static void gfx_fast_vline(gfx_t *gfx, gfx_coord_t start, gfx_coord_t end)
// {
//     if (start.x != end.x) {
//         return;
//     }
//     if (!gfx->stroke) {
//         return;
//     }
//     gfx->display->fns->region_set(gfx->display, (gfx_region_t){start.x, start.y, end.x, end.y});
//     gfx->display->fns->fill(gfx->display, gfx->strokeColour, end.y - start.y + 1);
// }

static void gfx_line_fill(gfx_t *gfx, gfx_coord_t start, gfx_coord_t end)
{
    if (start.x > end.x)
    {
        int16_t t = start.x;
        start.x = end.x;
        end.x = t;
        t = start.y;
        start.y = end.y;
        end.y = t;
    }
    MODULE_CALL_THIS(display, region_set, gfx->display, (gfx_region_t){start.x, start.y, end.x, end.y});
    MODULE_CALL_THIS(display, fill, gfx->display, gfx->fillColour, indirect_call(multiply)((uint32_t)(end.x - start.x + 1), (uint32_t)(end.y - start.y + 1)));
}

void gfx_line(gfx_t *gfx, gfx_coord_t start, gfx_coord_t end)
{
    // return;
    if (!gfx->stroke)
    {
        return;
    }

    int16_t steep = abs(end.y - start.y) > abs(end.x - start.x);
    if (steep)
    {
        int16_t t = start.x;
        start.x = start.y;
        start.y = t;
        t = end.x;
        end.x = end.y;
        end.y = t;
    }
    if (start.x > end.x)
    {
        int16_t t = start.x;
        start.x = end.x;
        end.x = t;
        t = start.y;
        start.y = end.y;
        end.y = t;
    }
    int16_t dx = end.x - start.x;
    int16_t dy = abs(end.y - start.y);
    int16_t err = dx / 2;
    int16_t ystep;

    if (start.y < end.y)
    {
        ystep = 1;
    }
    else
    {
        ystep = -1;
    }
    for (; start.x <= end.x; start.x++)
    {
        if (steep)
        {
            indirect_call(gfx_point)(gfx, (gfx_coord_t){start.y, start.x});
        }
        else
        {
            indirect_call(gfx_point)(gfx, (gfx_coord_t){start.x, start.y});
        }
        err -= dy;
        if (err < 0)
        {
            start.y += ystep;
            err += dx;
        }
    }
}

__attribute__((optimize("O3"))) void gfx_rectangle(gfx_t *gfx, gfx_region_t r)
{
    if (gfx->fill)
    {
        MODULE_CALL_THIS(display, region_set, gfx->display, r);
        MODULE_CALL_THIS(display, fill, gfx->display, gfx->fillColour, (uint32_t)(r.x2 - r.x1 + 1) * (uint32_t)(r.y2 - r.y1 + 1));
    }
    if (gfx->stroke)
    {
        gfx_line(gfx, (gfx_coord_t){r.x1, r.y1}, (gfx_coord_t){r.x2, r.y1});
        gfx_line(gfx, (gfx_coord_t){r.x2, r.y1}, (gfx_coord_t){r.x2, r.y2});
        gfx_line(gfx, (gfx_coord_t){r.x2, r.y2}, (gfx_coord_t){r.x1, r.y2});
        gfx_line(gfx, (gfx_coord_t){r.x1, r.y2}, (gfx_coord_t){r.x1, r.y1});
    }
}

void gfx_circle(gfx_t * /* gfx */, gfx_coord_t /* center */, uint8_t /* radius */) {}
void gfx_ellipse(gfx_t * /* gfx */, gfx_coord_t /* focusA */, gfx_coord_t /* focusB */, uint8_t /* radius */) {}
void gfx_triangle(gfx_t *gfx, gfx_coord_t a, gfx_coord_t b, gfx_coord_t c)
{

    if (a.y > b.y)
    {
        gfx_coord_t t = a;
        a = b;
        b = t;
    }
    if (a.y > c.y)
    {
        gfx_coord_t t = a;
        a = c;
        c = t;
    }
    if (b.y > c.y)
    {
        gfx_coord_t t = b;
        b = c;
        c = t;
    }

    int16_t dx1 = b.x - a.x;
    int16_t dy1 = b.y - a.y;
    int16_t dx2 = c.x - a.x;
    int16_t dy2 = c.y - a.y;
    int16_t dx3 = c.x - b.x;
    int16_t dy3 = c.y - b.y;

    for (int16_t y = a.y; y <= c.y; y++)
    {
        if (y < b.y)
        {
            int16_t x1 = a.x + (dx1 * (y - a.y)) / dy1;
            int16_t x2 = a.x + (dx2 * (y - a.y)) / dy2;
            if (x1 > x2)
            {
                int16_t t = x1;
                x1 = x2;
                x2 = t;
            }
            indirect_call(gfx_line_fill)(gfx, (gfx_coord_t){x1, y}, (gfx_coord_t){x2, y});
        }
        else
        {
            int16_t x1 = b.x + indirect_call(divide)(indirect_call(multiply)(dx3, (y - b.y)), dy3);
            int16_t x2 = a.x + indirect_call(divide)(indirect_call(multiply)(dx2, (y - a.y)), dy2);
            if (x1 > x2)
            {
                int16_t t = x1;
                x1 = x2;
                x2 = t;
            }
            indirect_call(gfx_line_fill)(gfx, (gfx_coord_t){x1, y}, (gfx_coord_t){x2, y});
        }
    }

    indirect_call(gfx_line)(gfx, a, b);
    indirect_call(gfx_line)(gfx, b, c);
    indirect_call(gfx_line)(gfx, c, a);
}
void gfx_arc(gfx_t * /* gfx */, gfx_coord_t /* center */, uint8_t /* radiusX */, uint8_t /* radiusY */, uint16_t /* startAngle */, uint16_t /* endAngle */) {}
void gfx_char(gfx_t *gfx, gfx_coord_t pos, char c)
{
    if (gfx->font == NULL)
    {
        return;
    }
    (void)pos;
    (void)c;


    display_region_t char_bounds = MODULE_CALL_THIS(font, char_get_bounds, gfx->font, c, gfx->textSize);
    uint8_t *buf = alloca((char_bounds.x2 - char_bounds.x1 + 1) * (char_bounds.y2 - char_bounds.y1 + 1));
    MODULE_CALL_THIS(font, get_char_pixels, gfx->font, c, char_bounds, buf, gfx->textSize);
    uint16_t pixel_count = 0;
    uint8_t i, j;
    for (j = char_bounds.y1; j <= char_bounds.y2; j++)
    {
        for (i = char_bounds.x1; i <= char_bounds.x2; i++)
        {
            uint16_t idx = (i - char_bounds.x1) + (j - char_bounds.y1) * (char_bounds.x2 - char_bounds.x1 + 1);
            if (buf[idx])
            // if (MODULE_CALL_THIS(font, get_char_pixel, gfx->font, c, (gfx_coord_t){i, j}, gfx->textSize))
            {
                pixel_count++;
            } else if (pixel_count) {
                gfx_region_t r = {
                    .x1 = pos.x + i-pixel_count+1,
                    .y1 = pos.y - j,
                    .x2 = pos.x + i,
                    .y2 = pos.y - j
                };
                MODULE_CALL_THIS(display, region_set, gfx->display, r);
                MODULE_CALL_THIS(display, fill, gfx->display, gfx->fillColour, pixel_count);
                pixel_count = 0;
            }
        }
        if (pixel_count) {
            gfx_region_t r = {
                .x1 = pos.x + i-pixel_count+1,
                .y1 = pos.y - j,
                .x2 = pos.x + i,
                .y2 = pos.y - j
            };
            MODULE_CALL_THIS(display, region_set, gfx->display, r);
            MODULE_CALL_THIS(display, fill, gfx->display, gfx->fillColour, pixel_count);
            pixel_count = 0;
        }
    }

    // indirect_call(gfx_point)(gfx, pos);

}
void gfx_text(gfx_t * gfx, gfx_region_t box, const char * text)
{
    gfx_coord_t pos = {box.x1, box.y1};
    for (uint8_t i = 0; text[i] != '\0'; i++)
    {
        indirect_call(gfx_char)(gfx, pos, text[i]);
        display_coord_t advance = MODULE_CALL_THIS(font, get_char_advance, gfx->font, text[i], gfx->textSize);
        pos.x += advance.x;
        pos.y += advance.y;
    }
}
void gfx_textP(gfx_t * gfx, gfx_region_t box, uint_farptr_t textP)
{
    gfx_coord_t pos = {box.x1, box.y1};
    for (char c; (c = pgm_read_byte_far(textP)); textP++)
    {
        indirect_call(gfx_char)(gfx, pos, c);
        display_coord_t advance = MODULE_CALL_THIS(font, get_char_advance, gfx->font, c, gfx->textSize);
        pos.x += advance.x;
        pos.y += advance.y;
    }
}
void gfx_textSize(gfx_t * gfx, uint16_t size)
{
    gfx->textSize = 48*size;
}
void gfx_textFont(gfx_t * gfx, font_t * font)
{
    gfx->font = font;
}

REGISTER_MODULE(gfx, GFX_MODULE_ID, GFX_FUNCTION_EXPORTS, 1);

