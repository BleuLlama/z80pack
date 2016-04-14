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
 */

extern BYTE imsai_fif_in(void);
extern BYTE imsai_fif_out(BYTE);
