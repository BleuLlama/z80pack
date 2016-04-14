/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2008-2014 by Udo Munk
 *
 * This modul contains initialization and reset functions for
 * the POSIX line discipline, so that stdin/stdout can be used
 * as terminal for ancient machines.
 *
 * History:
 * 24-SEP-08 first version finished
 * 16-JAN-14 discard input at reset
 */

#include <termios.h>

extern struct termios old_term, new_term;

extern void set_unix_terminal(void);
extern void reset_unix_terminal(void);
