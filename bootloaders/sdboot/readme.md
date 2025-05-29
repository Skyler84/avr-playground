# LaFortuna SD Bootloader

SDBoot allows interactively programming flash using binaries stored on a Micro-SD card.

## How it works

SDBoot is broken into two main components

### The Stub

the SDBoot stub is essentially a 1st stage bootloader. It bootstraps the main bootloader application. It has to contain SD card driver, FAT driver, dynamic module loader and boot programming functions. This enables it to find and program the second stage bootloader.

### The interactive portion

The SDBoot interactive 

## Functions

(WIP) SDBoot is able to bootstrap itself. The core bootloader (8KB) contains a minimal SD and FAT driver, flash programming functions, and is able to load the remaining portions of itself (modules) into application flash.
This includes the LCD, GFX, GUI and most of the interactive code. If an application requires all of the 120K space (or at least enough to overwrite parts of the extended bootloader), that is perfectly fine and SDBoot is able to bootstrap itself upon next initiation.

In the case of bootstrapping itself, the entire flash is erased?

## Technicalities

SDBoot is larger than the maximum bootloader size of 8KB, 4KWords, hence part of the application has to be kept outside of this limit.

Assuming 16KB, this gives us 112K application space. However, this means that some modules will be available to all programs!