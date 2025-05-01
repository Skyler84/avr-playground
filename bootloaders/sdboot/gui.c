#include "gui.h"
#include "display/display.h"
#include "module/pic.h"
#include "fonts/fonts.h"
#include "encoder.h"
#include "buttons.h"
#include <stdint.h>
#include <avr/pgmspace.h>


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

int8_t gui_msgboxP(GUI_t *gui, const char *msg, enum msgbox_type_t type)
{
    (void)gui;
    type = 0;
    display_xcoord_t boxw = 200;
    display_ycoord_t boxh = 80;
    display_region_t region = {
        .x1 = 160 - boxw / 2,
        .x2 = 160 + boxw / 2,
        .y1 = 120 - boxh / 2,
        .y2 = 120 + boxh / 2,
    };
    gui->display->fns->region_set(gui->display, region);
    gui->display->fns->fill(gui->display, 0x65bd, boxw * boxh);
    int len = my_strlen_P(msg);
    lcd_display_stringP(160 - 3 * len, 0, 100 - 4, 1, fonts_get_default(), &fonts_fns, msg, 0xFFFF);
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
        lcd_fill_rectangle(x - w / 2, x + w / 2, 140, 150, 0x65bd);
        if (btn_id > 7)
            continue;
        const char *s = pgm_read_word_elpm(&strings[btn_id]);
        lcd_display_stringP(x - 3 * my_strlen_P(s), 0, 141, 1, fonts_get_default(), &fonts_fns, s, 0xFFFF);
    }
    wait_button_click(0);
    return 0;
}

extern fonts_fns_t fonts_fns;

int8_t gui_choose_file(GUI_t *gui, FileSystem_t *fs, const char *path)
{
    (void)gui;
    FileInfo_t info;
    fstatus_t ret;
    int8_t selection = 0;
    uint8_t num_lines = 6;
    uint8_t line_start = 0;
    char dir[64] = "/FOLDER/";
    file_descriptor_t dirfd = fs->fns->opendir(&fs, dir);
    uint32_t cluster;
    const uint8_t line_spacing = 30;
    uint8_t selected = 0;
    while (1)
    {
        lcd_fill_rectangle(0, 320, 0, 20, BLACK);
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
            while (selected-- && (ret = fs->fns->getdents(&fs, dirfd, &info, 1)) == 1)
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
                file_descriptor_t newdirfd = fat_fns.opendirat(&fs, dirfd, info.name);
                if (newdirfd < 0)
                {
                    gui_msgboxP(gui,PSTR("Error opening directory"), MSGBOX_OK);
                    goto end;
                }
                fat_fns.closedir(&fs, dirfd);
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
            lcd_fill_rectangle(10, 309, ys, ye, colors[ln % 2]);
        }
        while ((ret = fat_fns.readdir(&fs, dirfd, &info)) == 0)
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
            lcd_display_string(40, 0, y, 2, fonts_get_default(), &fonts_fns, info.name, 0xFFFF);
            if (line_no == selection)
            {
                lcd_display_char(20, y, 2, fonts_get_default(), &fonts_fns, '>', WHITE);
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
