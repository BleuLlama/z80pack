# makefile to use Scott Lawrence's "Genroms" tool to build 
# rom files from the intel hex file

TARGROM := $(TARGBASE).rom

GENFILES := \
		$(TARGROM) \
		$(TARGBASE).lst \
		$(TARGBASE).ihx $(TARGBASE).hex \
		$(TARGBASE).rel $(TARGBASE).map 

################################################################################
# build rules

all: $(TARGROM) $(TARGBASE).hex romdirs
	@echo "+ copy $(TARGROM) to ROMs directory"
	@cp $(TARGROM) ../ROMs
	@cp $(TARGBASE).hex ../ROMs

$(TARGROM): $(TARGBASE).ihx
	@echo "+ genroms $<"
	@genroms ../Common/rc2014.roms $<
	@mv rc2014.rom $@

romdirs:
	@echo "+ Creating rom directory"
	@-mkdir ../ROMs

%.hex: %.ihx
	@echo "+ rename IHX as HEX"
	@cp $< $@

%.ihx: %.rel %.map
	@echo "+ aslink $@"
	@aslink -i -m -o $@ $<

%.rel %.map %.lst: %.asm
	@echo "+ asz80 $<"
	@asz80 -l $<

################################################################################

clean:
	@echo "+ Cleaning directory"
	@-rm $(GENFILES) 2>/dev/null || true
