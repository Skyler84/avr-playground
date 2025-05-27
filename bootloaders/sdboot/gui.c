#include "gui.h"
#include "display/display.h"
#include "module/imports.h"
#include "module/pic.h"
#include "font/font.h"
#include "gfx/gfx.h"
#include "encoder.h"
#include "buttons.h"
#include <stdint.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include <alloca.h>


static size_t my_strlen_P(uint32_t sP)
{
    size_t len = 0;
    char c;
    while ((c = pgm_read_byte_far(sP++)))
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

int8_t gui_msgbox(GUI_t *gui, const char* msg, enum msgbox_type_t type)
{
    (void)gui;
    (void)type;
    type = 0;
    display_xcoord_t boxw = 200;
    display_ycoord_t boxh = 80;
    gfx_region_t region = {
        .x1 = 160 - boxw / 2,
        .x2 = 160 + boxw / 2,
        .y1 = 120 - boxh / 2,
        .y2 = 120 + boxh / 2,
    };
    MODULE_CALL_THIS(gfx, fill, gui->gfx, BLUE);
    MODULE_CALL_THIS(gfx, nostroke, gui->gfx);
    MODULE_CALL_THIS(gfx, rectangle, gui->gfx, region);
    int len = strlen(msg);
    MODULE_CALL_THIS(gfx, fill, gui->gfx, WHITE);
    MODULE_CALL_THIS(gfx, textSize, gui->gfx, 12);
    MODULE_CALL_THIS(gfx, text, gui->gfx, ((display_region_t){160 - 3 * len, 130-boxh/3, 0, 0}), msg);
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
        MODULE_CALL_THIS(gfx, fill, gui->gfx, 0x65bd);
        MODULE_CALL_THIS(gfx, rectangle, gui->gfx, region);
        if (btn_id > 7)
            continue;
        const char *s = (const char*)pgm_read_word_elpm(&strings[btn_id]);
        MODULE_CALL_THIS(gfx, fill, gui->gfx, BLACK);
        MODULE_CALL_THIS(gfx, textP, gui->gfx, ((display_region_t){x-3*my_strlen_P(0x10000UL|(uintptr_t)s), 148, 0, 157}), 0x10000UL|(uintptr_t)s);
    }
    wait_button_click(BTN_C);
    return 0;
}

int8_t gui_msgboxP(GUI_t *gui, uint32_t msgP, enum msgbox_type_t type)
{
    (void)gui;
    (void)msgP;
    (void)type;
    size_t len = my_strlen_P(msgP);
    char buf[64];
    len = (len > (sizeof(buf) - 1)) ? (sizeof(buf) - 1) : len;
    for (size_t i = 0; i < len; i++)
    {
        buf[i] = pgm_read_byte_far(msgP + i);
    }
    return gui_msgbox(gui, buf, type);
}

file_descriptor_t gui_choose_file(GUI_t *gui, FileSystem_t *fs, const char *_dir)
{
    (void)gui;
    (void)fs;
    FileInfo_t info;
    fstatus_t ret;
    const uint8_t num_lines = 6;
    const uint8_t line_spacing = 180/num_lines;
    int8_t selection = 0;
    uint8_t line_start = 0;
    MODULE_CALL_THIS(gfx, textSize, gui->gfx, 144/num_lines);
    if (_dir == NULL)
    {
        _dir = "/";
    }
    char dir[256];;
    strncpy(dir, _dir, sizeof(dir));
    file_descriptor_t dirfd;
    dirfd = MODULE_CALL_THIS(fs, open, fs, dir, O_RDONLY | O_DIRECTORY);
    uint8_t selected = 0;
    while (1)
    {
        PORTB |= 0x80;
        int8_t line_no = 0;
        uint16_t colors[] = {0x457f, 0x0357, 0x1af2, 0x4438};
        if (selected)
        {
            // get FileInfo of selected item
            MODULE_CALL_THIS(fs, seek, fs, dirfd, 0, SEEK_SET);
            while (selected-- && (ret = MODULE_CALL_THIS(fs, getdirents, fs, dirfd, &info, 1)) == 1)
            {
                // do nothing
            }
            if (ret != 1)
            {
                static const char msg[] = "Error reading directory";
                uint_farptr_t msgP = pgm_get_far_address(msg);
                char buf[sizeof(msg)];
                char c;
                int i = 0;
                while((c = pgm_read_byte_far(msgP+i)))
                {
                    buf[i++] = c;
                }
                gui_msgbox(gui, buf, MSGBOX_OK);
                goto end;
            }
            if (info.type == 1)
            {
                file_descriptor_t fd = MODULE_CALL_THIS(fs, openat, fs, dirfd, info.name, O_RDONLY);
                MODULE_CALL_THIS(fs, close, fs, dirfd);
                return fd;
            }
            else if (info.type == 2)
            {
                // directory
                file_descriptor_t newdirfd = MODULE_CALL_THIS(fs, openat, fs, dirfd, info.name, O_RDONLY  | O_DIRECTORY);
                if (newdirfd < 0 || newdirfd == dirfd)
                {
                    char msg[] = "Error opening directory";
                    gui_msgbox(gui, msg, MSGBOX_OK);
                    goto end;
                }
                MODULE_CALL_THIS(fs, close, fs, dirfd);
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
        for (uint8_t ln = 0; ln < num_lines; ln++) {
            display_ycoord_t y = 40 + ln * line_spacing;
            display_ycoord_t ys = y - line_spacing / 4;
            display_ycoord_t ye = y + (line_spacing*3 + 1) / 4;
            MODULE_CALL_THIS(gfx, fill, gui->gfx, colors[(ln+line_start) % (sizeof(colors)/sizeof(colors[0]))]);
            MODULE_CALL_THIS(gfx, rectangle, gui->gfx, (display_region_t){10, ys, 309, ye});
            y += 15;
        }
        MODULE_CALL_THIS(fs, seek, fs, dirfd, 0, SEEK_SET);
        MODULE_CALL_THIS(gfx, fill, gui->gfx, WHITE);
        MODULE_CALL_THIS(fs, getdirents, fs, dirfd, NULL, 0);
        while ((ret = MODULE_CALL_THIS(fs, getdirents, fs, dirfd, &info, 1)) == 1)
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
            uint8_t ln = line_no - line_start;
            display_ycoord_t y = 40 + ln * line_spacing;
            y += line_spacing/2;
            char name[256];
            strncpy(name, info.name, sizeof(name));
            if (info.type == FI_TYPE_DIR)
            {
                strncat(name, "/", sizeof(name) - strlen(name) - 1);
            }
            MODULE_CALL_THIS(gfx, text, gui->gfx, ((display_region_t){40, y, 0, y}), name);
            if (line_no == selection)
            {
                MODULE_CALL_THIS(gfx, text, gui->gfx, ((display_region_t){20, y, 0, y}), ">");
            }
            line_no++;
        }
        PORTB &= ~0x80;
        
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
        
        // clamp selection to range
        if (selection < 0)
        {
            selection = line_no -1;
        }
        if (selection >= line_no)
        {
            selection = 0;
        }
        // move range based on selection
        if (selection >= line_start + num_lines - 1 && line_start < line_no - num_lines)
        {
            line_start = selection - num_lines + 2;
        }
        if (selection <= line_start && line_start > 0)
        {
            line_start = selection-1;
        }
        if (selection == 0) 
        {
            line_start = 0;
        }
        // clamp line_start
        if (line_start > line_no)
        {
            line_start = line_no - 1;
        }
        if (line_no > num_lines && line_start > line_no - num_lines)
        {
            line_start = line_no - num_lines;
        }
    }
end:
    MODULE_CALL_THIS(fs, close, fs, dirfd);
    return -1;
}
