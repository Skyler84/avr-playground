# Modules

AVR-GCC does not support position-independant-code (PIC), relocatables or shared libraries. 
This is due to the expectation that everything is statically linked into a single executable.

It was my goal to change this, to make position-independant "Modules" that could be loaded anywhere into flash, detected and run by an application. This allows several benefits such as being able to share code between bootloader and application.

## Problems

When writing PIC, absolute jumps/calls must be avoided at all costs. We can start by using `-mrelax`, which will instruct the linker to optimise jumps and calls that will fit into a relative jump/call instead. Occasionally this fails, and we have to resort to alternative methods.

## Module IDs

0x00** Reserved for specific modules such as bootloaders/dynamic loader. These are generally stored in bootloader section.
0x0000 Boot module - handles flash programming, reeboting etc.
0x0001 Fuses/Lock bits - handle reading(/writing) fuse and lock bits?
0x0010 Dynamic linker - Finds module functions at runtime.
0x0011 Dynamic loader - Loads modules into flash if not found
0x01** Modules by Skyler84
0x010* Display interfaces
0x0101 LCD display interface
0x011* Fonts
0x0111 font5x7 bitmamp font
0x012* Storage Block Devices
0x0120 Blockdev module?
0x0121 SD card driver
0x013* FileSystems
0x0130 VFS filesystem driver
0x0131 FAT filesystem driver
0x014* Graphics
0x015* Communications
0x0150 HW UART
0x0151 HW SPI
0x0152 HW I2C
0x0107 GUI module
0x0108 GFX module
0x01f0 Inputs module
