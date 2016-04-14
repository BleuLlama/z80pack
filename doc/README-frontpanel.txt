The original frontpanel library is available at:

	http://sourceforge.net/projects/frontpanel/

z80pack is integrated with a modified version of the released frontpanel
library for better portability, which includes bug fixes and a port for
Windows.

Also the frontpanel configuration files have small modifications from
the released version at sourceforge, due to somewhat different directory
structure of the packages.

To build the Altair 8800, IMSAI 8080 and Cromemco Z-1 emulations including
the frontpanel first change to directory ~/z80pack-x.y/frontpanel. There
you'll find several files Makefile.operating-system, to build the library
for your OS type:

make -f Makefile.linux		for a Linux system
make -f Makefile.osx		for an Apple OS X system
...

This will build the shared library libfrontpanel.so.

This library needs to be in a shared library path for the runtime linker,
so that the emulation programs can find this library. On my systems I
copy the library to /usr/local/lib and my ~/.profile includes:

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

I have seen systems where this won't work, on such systems I copied the
library to /usr/lib or /usr/lib64, so that it was found by the programs.

After you upgraded the library or the z80pack distribution this step
needs to be repeated, so that the latest version of the library is used.

After the library is in place build the simulated machines as follows:

cd ~/z80pack-x.y/altairsim/srcsim
make -f Makefile.operating.system
make -f Makefile.operating-system clean

cd ~/z80pack-x.y/imsaisim/srcsim
make -f Makefile.operating-system
make -f Makefile.operating-system clean

cd ~/z80pack-x.y/cromemco/srcsim
make -f Makefile.operating-system
make -f Makefile.operating-system clean

To run the systems change into directory ~/z80pack-x.y/imsaisim
and run the program imsasim. To load memory with the included
programs use imsaisim -xbasic8k.hex. To boot an operating system
run the included scripts like ./imsai-cpm13.
The IMSAI emulation by default is the 3D model, if you want the
2D model change the symbolic link to the 2D configuration as follows:

cd ~/z80pack-x.y/imsaisim
rm conf
ln -s conf_2d conf

The 3D model also can be switched between 2D and 3D with the v-key pressed
in the frontpanel window.

The Cromemco Z-1 frontpanel was derived from the IMSAI frontpanel,
so it comes with 2D and 3D models, here 2D is the default.

Running the Altair emulation is the same, just change to directory
~/z80pack-x.y/altairsim and run program altairsim. The Altair emulation
comes with 2D model only.

The default CPU speed for the Altair and IMSAI emulations is 2 MHz
and for the Cromemco emulation 4 Mhz, as with the original machines.
This can be changed with the command line option -fx, where x is the
desired CPU speed in MHz. With a value of 0 the CPU speed is unlimited
and the emulation runs as fast as possible.

The Cromemco emulation is sensitive for the speed setting, best is to
run it at the default 4 Mhz. Higher settings have an influence on
timings and some software won't run correct anymore.

Some functions of I/O devices such as the emulated SIO boards can be
configured in the configuration files:

~/z80pack-x.y/imsaisim/conf/iodev.conf
~/z80pack-x.y/altairsim/conf/iodev.conf

This configuration files include comments, usage of the options should
be obvious.
