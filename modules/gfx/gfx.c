#include "gfx/gfx.h"
#include "module/imports.h"

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
    return (gfx_colour_t)(((r&0xf8) << 8) | ((g&0xfc) << 3) | (b >> 3));
}

gfx_colour_t gfx_colourA(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
    return (gfx_colour_t)(((a&0xf8) << 8) | ((r&0xf8) << 8) | ((g&0xfc) << 3) | (b >> 3));
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
    if (!gfx->stroke) {
        return;
    }
    gfx_region_t r = {
        .x1 = coord.x - gfx->strokeWeight / 2,
        .y1 = coord.y - gfx->strokeWeight / 2,
        .x2 = coord.x + (gfx->strokeWeight - 1) / 2,
        .y2 = coord.y + (gfx->strokeWeight - 1) / 2,
    };
    gfx->display->fns->region_set(gfx->display, r);
    gfx->display->fns->fill(gfx->display, gfx->strokeColour, (uint32_t)gfx->strokeWeight * gfx->strokeWeight);

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
    if (start.x > end.x) {
        int16_t t = start.x;
        start.x = end.x;
        end.x = t;
        t = start.y;
        start.y = end.y;
        end.y = t;
    }
    gfx->display->fns->region_set(gfx->display, (gfx_region_t){start.x, start.y, end.x, end.y});
    gfx->display->fns->fill(gfx->display, gfx->fillColour, (uint32_t)(end.x - start.x + 1) * (uint32_t)(end.y - start.y + 1));
}

void gfx_line(gfx_t *gfx, gfx_coord_t start, gfx_coord_t end)
{
    if (!gfx->stroke) {
        return;
    }

    int16_t steep = abs(end.y - start.y) > abs(end.x - start.x);
    if (steep) {
        int16_t t = start.x;
        start.x = start.y;
        start.y = t;
        t = end.x;
        end.x = end.y;
        end.y = t;
    }
    if (start.x > end.x) {
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

    if (start.y < end.y) {
        ystep = 1;
    } else {
        ystep = -1;
    }
    for (; start.x <= end.x; start.x++) {
        if (steep) {
            gfx_point(gfx, (gfx_coord_t){start.y, start.x});
        } else {
            gfx_point(gfx, (gfx_coord_t){start.x, start.y});
        }
        err -= dy;
        if (err < 0) {
            start.y += ystep;
            err += dx;
        }
    }
}

void gfx_rectangle(gfx_t *gfx, gfx_region_t r)
{
    if (gfx->fill) {
        MODULE_CALL_THIS(display, region_set, gfx->display, r);
        MODULE_CALL_THIS(display, fill, gfx->display, gfx->fillColour, (uint32_t)(r.x2 - r.x1 + 1) * (uint32_t)(r.y2 - r.y1 + 1));
    }
    if (gfx->stroke) {
        gfx_line(gfx, (gfx_coord_t){r.x1, r.y1}, (gfx_coord_t){r.x2, r.y1});
        gfx_line(gfx, (gfx_coord_t){r.x2, r.y1}, (gfx_coord_t){r.x2, r.y2});
        gfx_line(gfx, (gfx_coord_t){r.x2, r.y2}, (gfx_coord_t){r.x1, r.y2});
        gfx_line(gfx, (gfx_coord_t){r.x1, r.y2}, (gfx_coord_t){r.x1, r.y1});
    }
}

void gfx_circle(gfx_t *gfx, gfx_coord_t center, uint8_t r) 
{
    uint16_t r2 = r*r;
    if (gfx->fill) {
        for (int16_t yoff = r-1; yoff > -r; yoff--) {
            int16_t xoff;
            for (xoff = 0; (uint16_t)(xoff*xoff + yoff*yoff) < r2; xoff++);
            xoff--;
            gfx_line_fill(gfx, (gfx_coord_t){center.x-xoff, center.y+yoff}, (gfx_coord_t){center.x+xoff, center.y+yoff});
        }
    }
}
void gfx_ellipse(gfx_t */* gfx */, gfx_coord_t /* focusA */, gfx_coord_t /* focusB */, uint8_t /* radius */) {}
void gfx_triangle(gfx_t *gfx, gfx_coord_t a, gfx_coord_t b, gfx_coord_t c) 
{

    if (a.y > b.y) {
        gfx_coord_t t = a;
        a = b;
        b = t;
    }
    if (a.y > c.y) {
        gfx_coord_t t = a;
        a = c;
        c = t;
    }
    if (b.y > c.y) {
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

    for (int16_t y = a.y; y <= c.y; y++) {
        if (y < b.y) {
            int16_t x1 = a.x + dx1 * (y - a.y) / dy1;
            int16_t x2 = a.x + dx2 * (y - a.y) / dy2;
            if (x1 > x2) {
                int16_t t = x1;
                x1 = x2;
                x2 = t;
            }
            gfx_line_fill(gfx, (gfx_coord_t){x1, y}, (gfx_coord_t){x2, y});
        } else {
            int16_t x1 = b.x + dx3 * (y - b.y) / dy3;
            int16_t x2 = a.x + dx2 * (y - a.y) / dy2;
            if (x1 > x2) {
                int16_t t = x1;
                x1 = x2;
                x2 = t;
            }
            gfx_line_fill(gfx, (gfx_coord_t){x1, y}, (gfx_coord_t){x2, y});
        }
    }

    gfx_line(gfx, a, b);
    gfx_line(gfx, b, c);
    gfx_line(gfx, c, a);
}
void gfx_arc(gfx_t */* gfx */, gfx_coord_t /* center */, uint8_t /* radiusX */, uint8_t /* radiusY */, uint16_t /* startAngle */, uint16_t /* endAngle */) {}
void gfx_text(gfx_t */* gfx */, gfx_region_t /* box */, const char */* text */) {}
void gfx_textP(gfx_t */* gfx */, gfx_region_t /* coord */, const char */* text */) {}
void gfx_textSize(gfx_t */* gfx */) {}
void gfx_textFont(gfx_t */* gfx */, font_t */* font */) {
    // gfx->font = font;
}



REGISTER_MODULE(gfx, GFX_MODULE_ID, GFX_FUNCTION_EXPORTS, 1);
