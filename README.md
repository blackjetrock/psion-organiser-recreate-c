# Psion Organiser Recreation C
# !!Work in progress!!

C based psion Organiser recreation code

This is an alternative for the code that runs on the Psion Organiser 2 recreation. It uses the standard RP Pico SDK to create the code that runs on the new organiser PCBs instead of the original code that emulated the original organiser processor (6303) and ROMs. This code is much much faster, but all the functionality that is needed has to be created from scratch.

This code is very much work in progress, I'm hacking the original emulatior based code and gradually removing what isn't needed.

This is the simple menu I've started with, there's a framework taken from the emulator code that runs a set of menus that are very similar to the original organiser code menus.

![IMG_20240810_140745952 (1)](https://github.com/user-attachments/assets/aa69ab7a-5603-4dbf-93de-effacbcaf2b0)

psion_recreate_tiny_2 is the latest code. This uses the TinyUSB library for USB and presents many USB functions. There's a CLI and a mass storage device and some other consoles.

It's easy to do graphics:

![IMG_20240810_140754626](https://github.com/user-attachments/assets/940a2ced-eca9-40f7-9196-6f506f1189c1)

There's no support for data storage yet, but the hardware has some serial EEPROMs (64K or 128K I think) and the Pico flash available. There's the gpios to the datapack slots too, of course, so some functionality from the datapack tool could be copied over. There's a USB menu, as well. If you build a recreation with a PicoW then you have Wifi and BT as well. I haven't done that yet.

NewOPL
======

Newopl translate and execute has been ported to the recreation. It requires an SD card and 8Mb of external PSRAM. I used a Pimoroni Pico Plus 2, anything similar should work.
The code runs in the RP2350 in a larger stack for both cores. A custom .ld script has been added to the code and that handles the different stack position. The SD card is used for log files, these are the same as the Linux version of NewOPL. The code is not common between Linux and the Pico2 as there are a lot of differences in file IO and so on. It may be possible later to merge the code back together, but for now it's separate.

New OPL commands
----------------

It's possible to add new commands to OPL, that is mainky a table update process. The parser and the rintime have tables of QCodes and OPl commands and addind entries to those allows new commands to be added. There is some code to add and a parser to update, but for simple syntax and commands that is minimal.
The Pico2 code here has these commands added:

* GCLS    Clears the display
* GPOINT  Plots a pixel
* GUPDATE Updates the display


