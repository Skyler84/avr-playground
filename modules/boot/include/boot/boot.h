#pragma once

#include <stdint.h>
#include "module/module.h"

typedef enum{
    REBOOT_MODE_APP,
    REBOOT_MODE_BOOTLOADER,
}reboot_mode_t;

#define BOOT_FUNCTION_EXPORTS(modname, o) \
    o(modname, reboot, void, reboot_mode_t reboot_mode) \
    o(modname, get_page_size, uint32_t) \
    o(modname, flash_erase, void, uint32_t page) \
    o(modname, flash_program, void, uint32_t page, const uint8_t *buf)

#define BOOT_MODULE_ID 0x0000
#define BOOT_API_VER 1

DECLARE_MODULE(boot, BOOT_MODULE_ID, BOOT_FUNCTION_EXPORTS);