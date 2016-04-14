CC = gcc

CFLAGS= -O -s -Wall

all: mkdskimg bin2hex send receive ptp2bin
	@echo "done"

mkdskimg: mkdskimg.c
	$(CC) $(CFLAGS) -o mkdskimg mkdskimg.c
	cp mkdskimg ~/bin

bin2hex: bin2hex.c
	$(CC) $(CFLAGS) -o bin2hex bin2hex.c
	cp bin2hex ~/bin 

send: send.c
	$(CC) $(CFLAGS) -o send send.c
	cp send ..

receive: receive.c
	$(CC) $(CFLAGS) -o receive receive.c
	cp receive ..

ptp2bin: ptp2bin.c
	$(CC) $(CFLAGS) -o ptp2bin ptp2bin.c
	cp ptp2bin ~/bin

clean:
	rm -f mkdskimg mkdskimg.exe bin2hex bin2hex.exe receive receive.exe \
	send send.exe ptp2bin ptp2bin.exe

allclean:
	make clean
	rm -f ../receive ../receive.exe ../send ../send.exe
