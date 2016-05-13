# rc2014sim (RC2014/LL Emulator)

This is a simulator/emulator for Spencer Owen's RC2014 project,
with additional changes by Scott Lawrence to add in a slightly
different memory map and additional bank switching hardware.

I have named my extensions "RC2014/LL", which I will call "LL" here
for short.

The code here is based on other Z80Pack simulations, but has a
few additional items:

- all ROMs are built from ASM source where possible
- BASIC versions are copied directly from Grant Searle's Z80 SBC project
- Loader for ROM burning
- sample "app" which does the required bank switching for general use

Also included are the sources for the various ROMs for the LL system.
The assembler used is "asz80" from the zcc package, which is available
in source form from my repository of Z80 dev tools available here:

    https://code.google.com/archive/p/bleu-romtools/

Also there is a required tool called "genroms" which converts Intel
hex files (IHX, HEX) to binary ROM files.
