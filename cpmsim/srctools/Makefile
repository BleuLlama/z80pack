CC = gcc

CFLAGS= -O -s -Wall

all: mkdskimg bin2hex send receive
	@echo "done"

mkdskimg: mkdskimg.c
	$(CC) $(CFLAGS) -o mkdskimg mkdskimg.c
	cp mkdskimg ..

bin2hex: bin2hex.c
	$(CC) $(CFLAGS) -o bin2hex bin2hex.c
	cp bin2hex ..

send: send.c
	$(CC) $(CFLAGS) -o send send.c
	cp send ..

receive: receive.c
	$(CC) $(CFLAGS) -o receive receive.c
	cp receive ..

clean:
	rm -f mkdskimg mkdskimg.exe bin2hex bin2hex.exe receive receive.exe \
	send send.exe

allclean:
	make clean
	rm -f ../mkdskimg ../mkdskimg.exe ../bin2hex ../bin2hex.exe \
	../receive ../receive.exe ../send ../send.exe
