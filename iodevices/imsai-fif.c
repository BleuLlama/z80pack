/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2014-2015 by Udo Munk
 *
 * Emulation of an IMSAI FIF S100 board
 *
 * This emulation was reverse engineered from running IMDOS on the machine,
 * and from reading the CP/M 1.3 BIOS and boot sources from IMSAI.
 * I do not have the manual for this board, so there still might be bugs.
 *
 * History:
 * 18-JAN-14 first working version finished
 * 02-MAR-14 improvements
 * 23-MAR-14 got all 4 disk drives working
 *    AUG-14 some improvements after seeing the original IMSAI CP/M 1.3 BIOS
 * 17-SEP-14 FDC error 9 for DMA overrun, as reported by Alan Cox for cpmsim
 * 27-JAN-15 unlink and create new disk file if format track command
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "sim.h"
#include "simglb.h"

/* offsets in disk descriptor */
#define DD_UNIT		0	/* unit/command */
#define DD_RESULT	1	/* result code */
#define DD_NN		2	/* track number high, not used */
#define DD_TRACK	3	/* track number low */
#define DD_SECTOR	4	/* sector number */
#define DD_DMAL		5	/* DMA address low */
#define DD_DMAH		6	/* DMA address high */

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

/* these are our disk drives */
static char *disks[4] = {
	"disks/drivea.dsk",
	"disks/driveb.dsk",
	"disks/drivec.dsk",
	"disks/drived.dsk"
};

BYTE imsai_fif_in(void)
{
	return(0);
}

void imsai_fif_out(BYTE data)
{
	static int fdstate = 0;		/* state of the fd */
	static int fdaddr[16];		/* address of disk descriptors */
	static int descno;		/* descriptor # */

	void disk_io(int);

	/*
	 * The controller understands these commands:
	 * 0x10: set address of a disk descriptor from the following two out's
	 * 0x00: do the work as setup in the disk descriptor
	 * 0x20: reset a drive to home position, the lower digit contains
	 *       the drive to reset, 0x2f for all drives
	 *
	 * The dd address only needs to be set once, the OS then can adjust
	 * the wanted I/O in the descriptor and send the 0x00 command
	 * multiple times for this descriptor.
	 *
	 * The commands 0x10 and 0x00 are OR'ed with a descriptor number
	 * 0x0 - 0xf, so there can be 16 different disk descriptors that
	 * need to be remembered.
	 */
	switch (fdstate) {
	case 0:	/* start of command phase */
		switch (data & 0xf0) {
		case 0x00:	/* do what disk descriptor says */
			descno = data & 0xf;
			disk_io(fdaddr[descno]);
			break;

		case 0x10:	/* next 2 is address of disk descriptor */
			descno = data & 0xf;
			fdstate++;
			break;
	
		case 0x20:	/* reset drive(s) */
			break;	/* no mechanical drives, so nothing to do */

		default:
			printf("FIF: unknown cmd %02x\r\n", data);
			return;
		}
		break;

	case 1: /* LSB of disk descriptor address */
		fdaddr[descno] = data;
		fdstate++;
		break;

	case 2: /* MSB of disk descriptor address */
		fdaddr[descno] += data << 8;
		fdstate = 0;
		break;

	default:
		printf("FIF: internal state error\r\n");
		cpu_error = IOERROR;
		cpu_state = STOPPED;
		break;
	}
}

/*
 *	Here we do the disk I/O.
 *
 *	The status byte in the disk descriptor is set as follows:
 *	1 - OK
 *	2 - illegal drive
 *	3 - no disk in drive
 *	4 - illegal track
 *	5 - illegal sector
 *	6 - seek error
 *	7 - read error
 *	8 - write error
 *	9 - DMA overrun > 0xffff
 *	15 - invalid command
 *
 *	These error codes will abort disk I/O, but this are not the ones
 *	the real controller would set. Without FIF manual the real error
 *	codes are not known yet.
 *	In the original IMSAI BIOS the upper 4 bits of the error code
 *	are described as "error class" and used as error code for CP/M.
 *	One error code is explicitely tested, so it is known:
 *		0A1H - drive not ready
 *	The IMSAI BIOS waits forever until the drive door is closed, for
 *	now I leave the used code as is, so that the IO is aborted.
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

	/* check for DMA overrun */
	if (dma_addr > 0xff80) {
		*(ram + addr + DD_RESULT) = 9;
		return;
	}

	/* convert IMSAI unit bit to internal disk no */
	switch (unit) {
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

	default: /* set error code for all other drives */
		 /* IMDOS sends unit 3 intermediate for drive C: & D: */
		 /* and the IMDOS format program sends unit 0 */
		*(ram + addr + DD_RESULT) = 2;
		return;
	}

	/* try to open disk image for the wanted operation */
	if (cmd == FMT_TRACK) {
		if (track == 0)
			unlink(disks[disk]);
		fd = open(disks[disk], O_RDWR|O_CREAT, 0644);
	} else
		fd = open(disks[disk], O_RDWR);
	if (fd == -1) {
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
		memset(&blksec, 0xe5, SEC_SZ);
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

	default:	/* unknown command */
		*(ram + addr + DD_RESULT) = 15;
		break;
	}

done:
	close(fd);
}
