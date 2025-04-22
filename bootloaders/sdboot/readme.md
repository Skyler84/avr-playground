# LaFortuna SD Bootloader

SDBoot is larger than the maximum bootloader size of 8KB, 4KWords, hence part of the application has to be kept outside of this limit.

Assuming 16KB, this gives us 112K application space. However, this means that some modules will be available to all programs!