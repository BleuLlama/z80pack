# z80pack for RC2014/LL

z80pack is a Zilog Z80 and Intel 8080 cross development package for
UNIX and Windows systems distributed with all sources under a BSD
style license

This fork of the z80pack adds support for emulation of Spencer
Owen's RC2014 project, which itself is an extension of Grant Searle's
Z80 32k and 56k SBC project.

The version emulated here is compatible with both of the above, but
adds some additional hardware to make it more like Grant's CPM
computer.  In particular it adds ROM toggle and bank switching in
16k byte blocks.

I have named my extensions "RC2014/LL", which I will call "LL" here
for short.

Also included are the sources for the various ROMs for the LL system.
The assembler used is "asz80" from the zcc package, which is available
in source form from my repository of Z80 dev tools available here:

    https://code.google.com/archive/p/bleu-romtools/

Also there is a required tool called "genroms" which converts Intel
hex files (IHX, HEX) to binary ROM files.

# Changes

Changes to the core Z80Pack emulator:

- sim1.c adds "POLL_IO" define checks.
-- this adds a routine for the extended code to poll any io necessary, since it may not be purely interrupt driven.
