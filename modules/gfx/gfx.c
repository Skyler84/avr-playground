#include "gfx/gfx.h"

void gfx_init(gfx_t *gfx, display_t *display)
{
    gfx->display = display;
    gfx->fill = true;
    gfx->stroke = true;
    gfx->strokeWeight = 1;
    gfx->strokeColour = BLACK;
    gfx->fillColour = WHITE;
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
    gfx->display->fns->set_pixel(gfx->display, coord, gfx->strokeColour);
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

void gfx_line(gfx_t *gfx, gfx_coord_t start, gfx_coord_t end)
{
    if (!gfx->stroke) {
        return;
    }
    gfx->display->fns->region_set(gfx->display, (gfx_region_t){start.x, start.y, end.x, end.y});
    gfx->display->fns->fill(gfx->display, gfx->strokeColour, 1);
}

void gfx_rectangle(gfx_t *gfx, gfx_region_t r)
{
    if (gfx->fill) {
        gfx->display->fns->region_set(gfx->display, r);
        gfx->display->fns->fill(gfx->display, gfx->fillColour, (r.x2 - r.x1 + 1) * (r.y2 - r.y1 + 1));
    }
    if (gfx->stroke) {
        gfx_line(gfx, (gfx_coord_t){r.x1, r.y1}, (gfx_coord_t){r.x2, r.y1});
        gfx_line(gfx, (gfx_coord_t){r.x2, r.y1}, (gfx_coord_t){r.x2, r.y2});
        gfx_line(gfx, (gfx_coord_t){r.x2, r.y2}, (gfx_coord_t){r.x1, r.y2});
        gfx_line(gfx, (gfx_coord_t){r.x1, r.y2}, (gfx_coord_t){r.x1, r.y1});
    }
}

void gfx_circle(gfx_t */* gfx */, gfx_coord_t /* center */, uint8_t /* radius */) {}
void gfx_ellipse(gfx_t */* gfx */, gfx_coord_t /* focusA */, gfx_coord_t /* focusB */, uint8_t /* radius */) {}
void gfx_triangle(gfx_t */* gfx */, gfx_coord_t /* a */, gfx_coord_t /* b */, gfx_coord_t /* c */, gfx_colour_t /* colour */) {}
void gfx_arc(gfx_t */* gfx */, gfx_coord_t /* center */, uint8_t /* radiusX */, uint8_t /* radiusY */, uint16_t /* startAngle */, uint16_t /* endAngle */) {}
void gfx_text(gfx_t */* gfx */, gfx_region_t /* box */, const char */* text */) {}
void gfx_textP(gfx_t */* gfx */, gfx_region_t /* coord */, const char */* text */) {}
void gfx_textSize(gfx_t */* gfx */) {}
void gfx_textFont(gfx_t */* gfx */, font_t */* font */) {
    // gfx->font = font;
}



REGISTER_MODULE(gfx, GFX_MODULE_ID, GFX_FUNCTION_EXPORTS, 1);
