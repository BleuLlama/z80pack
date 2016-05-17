# Makefile for RC2014/LL

SUBMAKE := Makefile.osx

all:	rc2014sim roms

ROMDIRS := \
	app_lloader \
	app_basic32 \
	app_basic56 \
	app_iotest


rc2014sim:
	@echo "Building emulator"
	cd srcsim ; make -f $(SUBMAKE)

roms: romdir
	@echo "Building ROMs"
	for dir in $(ROMDIRS); do \
               $(MAKE) -C $$dir; \
             done
romdir:
	@echo "Creating ROM directory"
	@-mkdir ROMs

clean:
	@echo "Cleaning up all products"
	-rm -rf ROMs
	cd srcsim ; make -f $(SUBMAKE) clean
	-rm rc2014sim
	for dir in $(ROMDIRS); do \
               $(MAKE) -C $$dir clean; \
             done

test:	all
	./rc2014sim
