/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2014 by Udo Munk
 *
 * Emulation of an IMSAI FIF S100 board
 *
 * This emulation was reverse engineered from running IMDOS on the machine.
 * I do not have the manual for this board, so there might be bugs.
 *
 * History:
 * 18-JAN-14 first working version finished
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "sim.h"
#include "simglb.h"

/* offsets in disk descriptor */
#define DD_UNIT		0
#define DD_RESULT	1
#define DD_NN		2
#define DD_TRACK	3
#define DD_SECTOR	4
#define DD_DMAL		5
#define DD_DMAH		6

/* FD command in disk descriptor unit/command field */
#define WRITE_SEC	1
#define READ_SEC	2
#define FMT_TRACK	3
#define VERIFY_DATA	4

/* 8" standard disks */
#define SEC_SZ		128
#define SPT		26
#define TRK		77

//#define DEBUG		/* so we can see what the FIF is asked to do */

/* this are our disk drives */
static char *disks[2] = {
	"disks/drivea.dsk",
	"disks/driveb.dsk"
};

BYTE imsai_fif_in(void)
{
	printf("FIF: input wanted?\r\n");
	return(0);
}

BYTE imsai_fif_out(BYTE data)
{
	static int fdstate = 0;		/* state of the fd */
	static int fdaddr[16];		/* address of disk descriptors */
	static int descno;		/* descriptor # */

	void disk_io(int);

	//printf("FIF i/o = %02x\r\n", data);

	/*
	 * The controller understands two commands:
	 * 0x10: set address of a disk descriptor from the following two out's
	 * 0x00: do the work as setup in the disk descriptor
	 *
	 * The address only needs to be set once, the OS then can adjust
	 * the wanted I/O in the descriptor and send the 0x00 command
	 * multiple times for this descriptor.
	 *
	 * The command is OR'ed with a descriptor number 0x0 - 0xf, so
	 * there can be 16 different disk decriptors that need to be
	 * remembered.
	 */
	switch (fdstate) {
	case 0:	/* start of command phase */
		switch (data & 0x10) {
		case 0x00:	/* do what disk descriptor says */
			descno = data & 0xf;
			disk_io(fdaddr[descno]);
			break;

		case 0x10:	/* next 2 is address of disk descriptor */
			descno = data & 0xf;
			fdstate++;
			break;
	
		default:
			printf("FIFO: cmd %02x?\r\n", data);
			return(0);
		}
		break;

	case 1: /* LSB of disk descriptor address */
		fdaddr[descno] = data;
		fdstate++;
		break;

	case 2: /* MSB of disk decriptor address */
		fdaddr[descno] += data << 8;
		fdstate = 0;
		break;

	default:
		printf("FIF: state error\r\n");
		cpu_error = IOERROR;
		cpu_state = STOPPED;
		break;
	}

	return(0);
}

void disk_io(int addr)
{
	register int i;
	static int fd;			/* fd for disk i/o */
	static long pos;		/* seek position */
	static int unit;		/* disk unit number */
	static int cmd;			/* disk command */
	static int track;		/* disk track */
	static int sector;		/* disk sector */
	static int dma_addr;		/* DMA address */
	static int disk;		/* internal disk no */
	static char blksec[SEC_SZ];

#ifdef DEBUG
	printf("FIF disk descriptor at %04x\r\n", addr);
	printf("FIF unit: %02x\r\n", *(ram + addr + DD_UNIT));
	printf("FIF result: %02x\r\n", *(ram + addr + DD_RESULT));
	printf("FIF nn: %02x\r\n", *(ram + addr + DD_NN));
	printf("FIF track: %02x\r\n", *(ram + addr + DD_TRACK));
	printf("FIF sector: %02x\r\n", *(ram + addr + DD_SECTOR));
	printf("FIF DMA low: %02x\r\n", *(ram + addr + DD_DMAL));
	printf("FIF DMA high: %02x\r\n", *(ram + addr + DD_DMAH));
	printf("\r\n");
#endif

	unit = *(ram + addr + DD_UNIT) & 0xf;
	cmd = *(ram + addr + DD_UNIT) >> 4;
	track = *(ram + addr + DD_TRACK);
	sector = *(ram + addr + DD_SECTOR);
	dma_addr = (*(ram + addr + DD_DMAH) * 256) + *(ram + addr + DD_DMAL);

	/* convert unit to internal disk no */
	switch (unit) {
	/*
	 * Disk unit 0 seems to be special somehow, the format program uses
	 * it to check if there are disks in drives. I don't know what the
	 * real controller would do with this command, but the result code 0
	 * here makes the format program working.
	 */
	case 0:
		*(ram + addr + DD_RESULT) = 0;
		return;

	case 1:	/* IMDOS drive A: */
		disk = 0;
		break;

	case 2:	/* IMDOS drive B: */
		disk = 1;
		break;

	default: /* fatal error for all other drives */
		 /* the OS sends unit 3 for drive C: and D: */
		 /* probably a bug, for now we use only 2 working drives */
		//printf("FIF: disk unit no. is %02x?\r\n", unit);
		*(ram + addr + DD_RESULT) = 2;
		return;
	}

	/* try to open disk image for the wanted operation */
	if ((fd = open(disks[disk], O_RDWR)) == -1) {
		//printf("FIF: no disk in unit %d\r\n", unit);
		*(ram + addr + DD_RESULT) = 2;
		return;
	}

	/* we have a disk, try wanted disk operation */
	switch(cmd) {
	case WRITE_SEC:
		pos = (track * SPT + sector - 1) * SEC_SZ;
		lseek(fd, pos, SEEK_SET);
		write(fd, ram + dma_addr, SEC_SZ);
		*(ram + addr + DD_RESULT) = 1;
		break;

	case READ_SEC:
		pos = (track * SPT + sector - 1) * SEC_SZ;
		lseek(fd, pos, SEEK_SET);
		read(fd, ram + dma_addr, SEC_SZ);
		*(ram + addr + DD_RESULT) = 1;
		break;

	case FMT_TRACK:
		memset(&blksec, 0, SEC_SZ);
		pos = track * SPT * SEC_SZ;
		lseek(fd, pos, SEEK_SET);
		for (i = 0; i < SPT; i++)
			write(fd, &blksec, SEC_SZ);
		*(ram + addr + DD_RESULT) = 1;
		break;

	case VERIFY_DATA: /* emulated disks are reliable, just report ok */
		*(ram + addr + DD_RESULT) = 1;
		break;

	default:
		printf("FIF: unknown cmd %02x\r\n", cmd);
		*(ram + addr + DD_RESULT) = 2;
		break;
	}


	close(fd);
}
