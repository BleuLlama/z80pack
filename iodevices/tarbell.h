/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2014 by Udo Munk
 *
 * Emulation of a Tarbell SD 1011D S100 board
 *
 * History:
 * 13-MAR-2014 first fully working version
 * 15-MAR-2014 some improvements for CP/M 1.3 & 1.4
 * 17-MAR-2014 close(fd) was missing in write sector lseek error case
 */

extern BYTE tarbell_stat_in(void), tarbell_track_in(void);
extern BYTE tarbell_sec_in(void), tarbell_data_in(void);
extern BYTE tarbell_wait_in(void);
extern void tarbell_cmd_out(BYTE), tarbell_track_out(BYTE);
extern void tarbell_sec_out(BYTE), tarbell_data_out(BYTE);
extern void tarbell_ext_out(BYTE);
