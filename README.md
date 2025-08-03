# Psion Organiser Recreation C
# !!Work in progress!!

C based psion Organiser recreation code

This is an alternative for the code that runs on the Psion Organiser 2 recreation. It uses the standard RP Pico SDK to create the code that runs on the new organiser PCBs instead of the original code that emulated the original organiser processor (6303) and ROMs. This code is much much faster, but all the functionality that is needed has to be created from scratch.

This code is very much work in progress, I'm hacking the original emulator based code and gradually removing what isn't needed.

This is the simple menu I've started with, there's a framework taken from the emulator code that runs a set of menus that are very similar to the original organiser code menus.

<img src="https://github.com/user-attachments/assets/aa69ab7a-5603-4dbf-93de-effacbcaf2b0" width="600">

psion_recreate_tiny_2 is the latest code. This uses the TinyUSB library for USB and presents many USB functions. There's a CLI and a mass storage device and some other consoles.

It's easy to do graphics:

<img src="https://github.com/user-attachments/assets/940a2ced-eca9-40f7-9196-6f506f1189c1" width="600">

There's no support for data storage yet, but the hardware has some serial EEPROMs (64K or 128K I think) and the Pico flash available. There's the gpios to the datapack slots too, of course, so some functionality from the datapack tool could be copied over. There's a USB menu, as well. If you build a recreation with a PicoW then you have Wifi and BT as well. I haven't done that yet.

Pico2
=====

I've moved to the Pico 2, specifically the Pimoroni Pico Plus 2 as it has an external 8MB PSRAM IC on board. this is needed for the port of NewOPL to the Pico. I've basically moved the PC version of the translator and runtime to the Pico. As that is a recursive descent parsrr it needs more than the default 4 or 8K stack that the standard Pico 2 provides. The tyranslator and runtime both write log files and I have kept those, they are written to the SD card. So, to use NewOPl on the Pico, you need a Pico with 8Mb of PSRAM and an SD card. There is still a Pico version of the code that has no NewOPL.

The code runs on a Psion Mini fitted with the Pico Plus 2:


<img src="https://github.com/user-attachments/assets/fb29606a-36d0-4813-8abe-4f0acc360326" width="600">


NewOPL
======

Newopl translate and execute has been ported to the recreation. It requires an SD card and 8Mb of external PSRAM. I used a Pimoroni Pico Plus 2, anything similar should work.
The code runs in the RP2350 in a larger stack for both cores. A custom .ld script has been added to the code and that handles the different stack position. The SD card is used for log files, these are the same as the Linux version of NewOPL. The code is not common between Linux and the Pico2 as there are a lot of differences in file IO and so on. It may be possible later to merge the code back together, but for now it's separate.

New OPL commands
----------------

It's possible to add new commands to OPL, that is mainly a table update process. The parser and the rintime have tables of QCodes and OPl commands and addind entries to those allows new commands to be added. There is some code to add and a parser to update, but for simple syntax and commands that is minimal.
The Pico2 code here has these commands added:

* GCLS    Clears the display
* GPOINT  Plots a pixel
* GUPDATE Updates the display


