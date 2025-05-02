#include "gui.h"
#include "display/display.h"
#include "module/imports.h"
#include "module/pic.h"
#include "fonts/fonts.h"
#include "gfx/gfx.h"
#include "encoder.h"
#include "buttons.h"
#include <stdint.h>
#include <avr/pgmspace.h>
#include <util/delay.h>


static size_t my_strlen_P(const char __flash1 *s)
{
    size_t len = 0;
    while (*s++)
    {
        len++;
    }
    return len;
}

const char s_ok[] PROGMEM = "OK";
const char s_cancel[] PROGMEM = "Cancel";
const char s_yes[] PROGMEM = "Yes";
const char s_no[] PROGMEM = "No";
const char s_retry[] PROGMEM = "Retry";
const char s_continue[] PROGMEM = "Continue";
const char s_abort[] PROGMEM = "Abort";
const char s_ignore[] PROGMEM = "Ignore";

const char * const strings[] PROGMEM = {
    s_ok,
    s_cancel,
    s_yes,
    s_no,
    s_retry,
    s_continue,
    s_abort,
    s_ignore,
};

const uint8_t btn_strings[7][3] PROGMEM = {
    {0, -1, -1},
    {0, 1, -1},
    {2, 3, 1},
    {2, 3, -1},
    {4, 1, -1},
    {1, 4, 5},
    {6, 4, 7},
};

void gui_init(GUI_t *gui)
{
    (void)gui;
    // initialize buttons
    DDRE &= ~0x80;
    PORTE |= 0x80;
    DDRC &= ~0x3C;
    PORTC |= 0x3C;

    // initialize encoder
    encoder_init();
}

#define pgm_read_byte_elpm(addr)   \
(__extension__({                \
    uint16_t __addr16 = (uint16_t)(addr); \
    uint8_t __result;           \
    __asm__ __volatile__        \
    (                           \
        "elpm" "\n\t"           \
        "mov %0, r0" "\n\t"     \
        : "=r" (__result)       \
        : "z" (__addr16)        \
        : "r0"                  \
    );                          \
    __result;                   \
}))

#define pgm_read_word_elpm(addr)         \
(__extension__({                            \
    uint16_t __addr16 = (uint16_t)(addr);   \
    uint16_t __result;                      \
    __asm__ __volatile__                    \
    (                                       \
        "elpm"           "\n\t"              \
        "mov %A0, r0"   "\n\t"              \
        "adiw r30, 1"   "\n\t"              \
        "elpm"           "\n\t"              \
        "mov %B0, r0"   "\n\t"              \
        : "=r" (__result), "=z" (__addr16)  \
        : "1" (__addr16)                    \
        : "r0"                              \
    );                                      \
    __result;                               \
}))

int8_t gui_msgboxP(GUI_t *gui, const char */* msg */, enum msgbox_type_t type)
{
    (void)gui;
    type = 0;
    display_xcoord_t boxw = 200;
    display_ycoord_t boxh = 80;
    gfx_region_t region = {
        .x1 = 160 - boxw / 2,
        .x2 = 160 + boxw / 2,
        .y1 = 120 - boxh / 2,
        .y2 = 120 + boxh / 2,
    };
    MODULE_CALL(gfx, fill, gui->gfx, 0x0000);
    MODULE_CALL(gfx, nostroke, gui->gfx);
    MODULE_CALL(gfx, rectangle, gui->gfx, region);
    // int len = my_strlen_P(msg);
    // lcd_display_stringP(160 - 3 * len, 0, 100 - 4, 1, fonts_get_default(), &fonts_fns, msg, 0xFFFF);
    uint8_t btn_count = 0;
    for (uint8_t i = 0; i < 3; i++)
    {
        if (pgm_read_byte_elpm(&btn_strings[type][i]) != 0xFF)
        {
            btn_count++;
        }
    }
    uint8_t spacing = 65;
    uint8_t w = 60;

    for (uint8_t i = 0; i < btn_count; i++)
    {
        uint8_t x = 160 - (btn_count - 1) * spacing / 2 + i * spacing;
        uint8_t btn_id = pgm_read_byte_elpm(&btn_strings[type][i]);
        display_region_t region = {
            .x1 = x - w / 2,
            .x2 = x + w / 2,
            .y1 = 140,
            .y2 = 150,
        };
        MODULE_CALL(gfx, fill, gui->gfx, 0x65bd);
        MODULE_CALL(gfx, rectangle, gui->gfx, region);
        if (btn_id > 7)
            continue;
        const char *s = (const char*)pgm_read_word_elpm(&strings[btn_id]);
        MODULE_CALL(gfx, stroke, gui->gfx, BLACK);
        MODULE_CALL(gfx, textP, gui->gfx, ((display_region_t){x-3*my_strlen_P(s), 141, 0, 150}), s);
    }
    wait_button_click(0);
    return 0;
}

extern fonts_fns_t fonts_fns;

int8_t gui_choose_file(GUI_t *gui, FileSystem_t *fs, const char */* path */)
{
    (void)gui;
    FileInfo_t info;
    fstatus_t ret;
    int8_t selection = 0;
    uint8_t num_lines = 6;
    uint8_t line_start = 0;
    char dir[64] = "/FOLDER/";
    file_descriptor_t dirfd = MODULE_CALL(fs, open, fs, dir, O_RDONLY | O_DIRECTORY);
    // uint32_t cluster;
    const uint8_t line_spacing = 30;
    uint8_t selected = 0;
    while (1)
    {
        MODULE_CALL(gfx, fill, gui->gfx, BLACK);
        MODULE_CALL(gfx, nostroke, gui->gfx);
        MODULE_CALL(gfx, rectangle, gui->gfx, (display_region_t){0, 320, 0, 20});
        // if (line_start > )
        // lcd_debug_u16(0, 0, line_start);
        // lcd_debug_u16(0, 10, selection);
        display_ycoord_t y = 40;
        int8_t line_no = 0;
        uint16_t colors[] = {0x045c, 0x12d1};
        if (selected)
        {
            // get FileInfo of selected item
            fs->fns->seek(&fs, dirfd, 0);
            while (selected-- && (ret = fs->fns->getdirents(&fs, dirfd, &info, 1)) == 1)
            {
                // do nothing
            }
            if (ret != 0)
            {
                // error reading directory
                gui_msgboxP(gui,PSTR("Error reading directory"), MSGBOX_OK);
                goto end;
            }
            if (info.type == 1)
            {

                selected = 0;
            }
            else if (info.type == 2)
            {
                // directory
                file_descriptor_t newdirfd = MODULE_CALL(fs, openat, fs, dirfd, info.name, O_RDONLY  | O_DIRECTORY);
                if (newdirfd < 0)
                {
                    gui_msgboxP(gui,PSTR("Error opening directory"), MSGBOX_OK);
                    goto end;
                }
                MODULE_CALL(fs, close, fs, dirfd);
                dirfd = newdirfd;
                if (dirfd < 0)
                {
                    goto end;
                }
                selected = 0;
                selection = 0;
                line_start = 0;
            }
            else
            {
                // unknown type
                goto end;
            }
        }
        fs->fns->seek(&fs, dirfd, 0);
        for (uint8_t ln = 0; ln < num_lines; ln++)
        {
            display_ycoord_t y = 40 + ln * line_spacing;
            display_ycoord_t ys = y - (line_spacing - 16) / 2;
            display_ycoord_t ye = y + (line_spacing + 1 + 16) / 2;
            MODULE_CALL(gfx, fill, gui->gfx, colors[ln % 2]);
            MODULE_CALL(gfx, rectangle, gui->gfx, (display_region_t){10, 309, ys, ye});
        }
        while ((ret = MODULE_CALL(fs, getdirents, fs, dirfd, &info, 1)) == 1)
        {
            if (line_no < line_start)
            {
                line_no++;
                continue;
            }
            if (line_no >= line_start + num_lines)
            {
                line_no++;
                continue;
            }
            MODULE_CALL(gfx, stroke, gui->gfx, WHITE);
            MODULE_CALL(gfx, text, gui->gfx, ((display_region_t){40, y, 0, y}), info.name);
            if (line_no == selection)
            {
                MODULE_CALL(gfx, text, gui->gfx, ((display_region_t){20, y, 0, y}), ">");
            }
            y += 30;
            line_no++;
        }
        while (1)
        {
            _delay_ms(10);
            int8_t dt = encoder_dt(0);
            if (button_clicked(BTN_N))
            {
                selection--;
                break;
            }
            if (button_clicked(BTN_S))
            {
                selection++;
                break;
            }
            if (button_clicked(BTN_C))
            {
                // button pressed
                selected = selection + 1;
                break;
            }
            if (dt < 2 && dt > -2)
            {
                continue;
            }
            int8_t sign = dt > 0 ? 1 : -1;
            encoder_dt(dt);
            selection += sign;
            break;
        }

        if (selection < 0)
        {
            selection = 0;
        }
        if (selection >= line_no)
        {
            selection = line_no - 1;
        }
        if (selection > line_start + num_lines - 1)
        {
            line_start = selection - num_lines + 1;
        }
        if (selection < line_start)
        {
            line_start = selection;
        }
        if (line_start > line_no)
        {
            line_start = line_no - 1;
        }
    }
end:
    fs->fns->close(&fs, dirfd);
    return 0;
}
