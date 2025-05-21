#pragma once

#include "module/module.h"

typedef struct GUI GUI_t;

#define GUI_FUNCTION_EXPORTS(modname, o) \
    o(modname, init, void, GUI_t *gui) \
    o(modname, msgboxP, int8_t, GUI_t *gui, uint32_t msgP, enum msgbox_type_t type) \
    o(modname, msgbox, int8_t, GUI_t *gui, const char* msg, enum msgbox_type_t type) \
    o(modname, choose_file, int8_t, GUI_t *gui, FileSystem_t *fs, const char *path)

#define GUI_MODULE_ID 0x0107
#define GUI_API_VER 1

DECLARE_MODULE(gui, GUI_MODULE_ID, GUI_FUNCTION_EXPORTS);
struct GUI{
    gfx_t *gfx;
};