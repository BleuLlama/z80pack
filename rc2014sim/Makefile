# Makefile for RC2014/LL

SUBMAKE := Makefile.osx

all:	rc2014sim roms


rc2014sim:
	@echo "Building emulator"
	cd srcsim ; make -f $(SUBMAKE)

roms: romdir
	@echo "Building ROMs"
	cd app_iotest ; make
	cd app_lloader ; make
	cd app_basic32 ; make
	cd app_basic56 ; make

romdir:
	@echo "Creating ROM directory"
	@-mkdir ROMs

clean:
	@echo "Cleaning up all products"
	-rm -rf ROMs
	cd srcsim ; make -f $(SUBMAKE) clean
	-rm rc2014sim
	cd app_iotest ; make clean
	cd app_lloader ; make clean
	cd app_basic32 ; make clean
	cd app_basic56 ; make clean
