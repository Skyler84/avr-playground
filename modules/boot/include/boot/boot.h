#pragma once

#include <stdint.h>

#define BOOT_FUNCTION_EXPORTS(modname, o) \
    o(modname, reboot, void, reboot_mode) \
    o(modname, get_page_size, uint32_t) \
    o(modname, flash_erase, void, uint32_t page) \
    o(modname, flash_program, void, uint32_t page, const uint8_t *buf)

extern void flash_page_erase(uint32_t page);
extern void flash_page_program(uint32_t page, const uint8_t *buf);