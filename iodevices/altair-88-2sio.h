/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2008-2014 by Udo Munk
 *
 * Partial emulation of an Altair 88-2SIO S100 board
 *
 * History:
 * 20-OCT-08 first version finished
 * 31-JAN-14 use correct name from the manual
 * 19-JUN-14 added config parameter for droping nulls after CR/LF
 * 17-JUL-14 don't block on read from terminal
 * 09-OCT-14 modified to support 2 SIO's
 */

extern BYTE altair_sio1_status_in(void);
extern void altair_sio1_status_out(BYTE);
extern BYTE altair_sio1_data_in(void);
extern void altair_sio1_data_out(BYTE);
