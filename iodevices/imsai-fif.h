/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2014-2015 by Udo Munk
 *
 * Emulation of an IMSAI FIF S100 board
 *
 * History:
 * 18-JAN-14 first version finished
 * 02-MAR-14 improvements
 * 23-MAR-14 got all 4 disk drives working
 *    AUG-14 some improvements after seeing the original IMSAI BIOS
 * 27-JAN-15 unlink and create new disk file if format track command
 */

extern BYTE imsai_fif_in(void);
extern void imsai_fif_out(BYTE);
