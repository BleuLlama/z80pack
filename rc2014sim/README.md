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

# To use

To use this, i have provided a precompiled ROM file, built from the
provided
.HEX files. The emulator will load this ROM on startup.

The makefile in this directory will build all of the roms.  If you
have my bleu-romtools installed as above, it will assemble some
roms, and generate ROM files for others. This is not required for
the project, but might be a nice thing to build everything from
scratch.  The makefile in this directory should do all of that for
you.

     make
     ls ROMs

To build just the emulator, go into the 'srcsim' directory.  In
there you will see the additions/changes to the z80pack project to
build  the RC2014 stuff (and some other stuff for my /LL project,
namely SD support via UART control, which will be pursued another
time.  (If you change the boot rom define in simctl.c to
"ROMs/lloader.rom" you'll see a preliminary version of this, where
it starts up with a simple file bootloader, then lets you load one
of the other built ROMs.  This project using RAM bankswitching is
incomplete at this time, and should be considered a prototype/experiment.

ANYWAY....

You will need to specify the makefile for your system.  I've only
extensively used the "Makefile.osx" makefile, since that's what I'm
using here.  If another makefile is not working for you, you may
need to tweak your appropriate makefile, using the .osx one as
reference.  If you have further problems, get a mac. ;)

    make -f Makefile.osx

It puts the binary up a directory, next to the ROMs folder, so change
to there, and run it.  There is no way to exit the emulator without 
killing it from another shell window. Sorry about that.

    cd ../
    ./rc2014sim

That should be it.  Enjoy!

-Scott Lawrence (yorgle@gmail.com)


