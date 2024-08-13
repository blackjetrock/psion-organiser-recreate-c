# Psion Organiser Recreation C
# !!Work in progress!!

C based psion Organiser recreation code

This is an alternative for the code that runs on the Psion Organiser 2 recreation. It uses the standard RP Pico SDK to create the code that runs on the new organiser PCBs instead of the original code that emulated the original organiser processor (6303) and ROMs. This code is much much faster, but all the functionality that is needed has to be created from scratch.

This code is very much work in progress, I'm hacking the original emulatior based code and gradually removing what isn't needed.

This is the simple menu I've started with, there's a framework taken from the emulator code that runs a set of menus that are very similar to the original organiser code menus.

![IMG_20240810_140745952 (1)](https://github.com/user-attachments/assets/aa69ab7a-5603-4dbf-93de-effacbcaf2b0)


It's easy to do graphics:

![IMG_20240810_140754626](https://github.com/user-attachments/assets/940a2ced-eca9-40f7-9196-6f506f1189c1)

There's no support for data storage yet, but the hardware has some serial EEPROMs (64K or 128K I think) and the Pico flash available. There's the gpios to the datapack slots too, of course, so some functionality from the datapack tool could be copied over. There's a USB menu, as well. If you build a recreation with a PicoW then you have Wifi and BT as well. I haven't done that yet.
