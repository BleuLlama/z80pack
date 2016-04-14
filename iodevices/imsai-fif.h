/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2014 by Udo Munk
 *
 * Emulation of an IMSAI FIF S100 board
 *
 * History:
 * 18-JAN-14 first version finished
 * 02-MAR-14 improvements
 * 23-MAR-14 got all 4 disk drives working
 */

extern BYTE imsai_fif_in(void);
extern void imsai_fif_out(BYTE);
