# Modules

AVR-GCC does not support position-independant-code (PIC), relocatables or shared libraries. 
This is due to the expectation that everything is statically linked into a single executable.

It was my goal to change this, to make position-independant "Modules" that could be loaded anywhere into flash, detected and run by an application. This allows several benefits such as being able to share code between bootloader and application.

## Problems

When writing PIC, absolute jumps/calls must be avoided at all costs. We can start by using `-mrelax`, which will instruct the linker to optimise jumps and calls that will fit into a relative jump/call instead. Occasionally this fails, and we have to resort to alternative methods.