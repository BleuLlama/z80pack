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
 * 02-MAR-14 improvements
 * 23-MAR-14 got all 4 disk drives working
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
static char *disks[4] = {
	"disks/drivea.dsk",
	"disks/driveb.dsk",
	"disks/drivec.dsk",
	"disks/drived.dsk"
};

BYTE imsai_fif_in(void)
{
	//printf("FIF: input wanted?\r\n");
	return(0);
}

void imsai_fif_out(BYTE data)
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
			return;
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
}

/*
 *	Here we do the disk I/O.
 *
 *	The status byte in the disk descriptor is set as follows:
 *	0 - ok
 *	1 - ok
 *	2 - illegal drive
 *	3 - no disk in drive
 *	4 - illegal track
 *	5 - illegal sector
 *	6 - seek error
 *	7 - read error
 *	8 - write error
 *	15 - other error
 *
 *	The error codes will abort disk I/O, but I don't know which
 *	error codes the real controller would set.
 */
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

	case 4: /* IMDOS drive C: */
		disk = 2;
		break;

	case 8: /* IMDOS drive D: */
		disk = 3;
		break;

	default: /* ignore all other drives */
		 /* IMDOS sends unit 3 intermediant for drive C: & D: */
		*(ram + addr + DD_RESULT) = 0;
		return;
	}

	/* try to open disk image for the wanted operation */
	if ((fd = open(disks[disk], O_RDWR)) == -1) {
		//printf("FIF: no disk in unit %d\r\n", unit);
		*(ram + addr + DD_RESULT) = 3;
		return;
	}

	/* we have a disk, try wanted disk operation */
	switch(cmd) {
	case WRITE_SEC:
		if (track >= TRK) {
			*(ram + addr + DD_RESULT) = 4;
			goto done;
		}
		if (sector > SPT) {
			*(ram + addr + DD_RESULT) = 5;
			goto done;
		}
		pos = (track * SPT + sector - 1) * SEC_SZ;
		if (lseek(fd, pos, SEEK_SET) == -1L) {
			*(ram + addr + DD_RESULT) = 6;
			goto done;
		}
		if (write(fd, ram + dma_addr, SEC_SZ) != SEC_SZ) {
			*(ram + addr + DD_RESULT) = 8;
			goto done;
		}
		*(ram + addr + DD_RESULT) = 1;
		break;

	case READ_SEC:
		if (track >= TRK) {
			*(ram + addr + DD_RESULT) = 4;
			goto done;
		}
		if (sector > SPT) {
			*(ram + addr + DD_RESULT) = 5;
			goto done;
		}
		pos = (track * SPT + sector - 1) * SEC_SZ;
		if (lseek(fd, pos, SEEK_SET) == -1L) {
			*(ram + addr + DD_RESULT) = 6;
			goto done;
		}
		if (read(fd, ram + dma_addr, SEC_SZ) != SEC_SZ) {
			*(ram + addr + DD_RESULT) = 7;
			goto done;
		}
		*(ram + addr + DD_RESULT) = 1;
		break;

	case FMT_TRACK:
		memset(&blksec, 0, SEC_SZ);
		if (track >= TRK) {
			*(ram + addr + DD_RESULT) = 4;
			goto done;
		}
		pos = track * SPT * SEC_SZ;
		if (lseek(fd, pos, SEEK_SET) == -1L) {
			*(ram + addr + DD_RESULT) = 6;
			goto done;
		}
		for (i = 0; i < SPT; i++) {
			if (write(fd, &blksec, SEC_SZ) != SEC_SZ) {
				*(ram + addr + DD_RESULT) = 8;
				goto done;
			}
		}
		*(ram + addr + DD_RESULT) = 1;
		break;

	case VERIFY_DATA: /* emulated disks are reliable, just report ok */
		*(ram + addr + DD_RESULT) = 1;
		break;

	default:
		//printf("FIF: unknown cmd %02x\r\n", cmd);
		*(ram + addr + DD_RESULT) = 15;
		break;
	}

done:
	close(fd);
}
