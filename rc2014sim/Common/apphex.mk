# makefile to use Scott Lawrence's "Genroms" tool to build 
# rom files from the intel hex file

TARGROM := $(TARGBASE).rom
SOURCEHEX := $(HEXBASE).hex

GENFILES := $(TARGROM) 

################################################################################
# build rules

all: $(TARGROM) $(HEXBASE).hex romdirs
	@echo "+ copy $(TARGROM) to ROMs directory"
	@cp $(TARGROM) ../ROMs
	@cp $(SOURCEHEX) ../ROMs/$(TARGBASE).hex

$(TARGROM): $(SOURCEHEX)
	@echo "+ genroms $<"
	@genroms ../Common/rc2014.roms $<
	@mv rc2014.rom $@

romdirs:
	@echo "+ Creating rom directory"
	@-mkdir ../ROMs

################################################################################

clean:
	@echo "+ Cleaning directory"
	@-rm $(GENFILES) 2>/dev/null || true
