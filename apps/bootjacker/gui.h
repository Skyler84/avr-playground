#pragma once

#include "fs/fs.h"
#include "gfx/gfx.h"

typedef struct GUI{
    gfx_t *gfx;
} GUI_t;

enum msgbox_type_t {
    MSGBOX_OK = 0,
    MSGBOX_OKCANCEL,
    MSGBOX_YESNOCANCEL,
    MSGBOX_YESNO,
    MSGBOX_RETRYCANCEL,
    MSGBOX_CANCELRETRYCONTINUE,
    MSGBOX_ABORTRETRYIGNORE,
};
    


extern void gui_init(GUI_t *gui);
extern int8_t gui_msgboxP(GUI_t *gui, uint32_t msgP, enum msgbox_type_t type);
extern int8_t gui_msgbox(GUI_t *gui, const char* msg, enum msgbox_type_t type);
extern int8_t gui_choose_file(GUI_t *gui, FileSystem_t *fs, const char *path);