/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2008-2014 by Udo Munk
 *
 * This module reads the I/O configuration file and sets
 * global variables for the I/O device emulations, so
 * that functions of I/O devices can be configured.
 *
 * History:
 * 20-OCT-08 first version finished
 * 20-MAR-14 ignore carriage return too, neccessary for the Windows port
 */

extern void io_config(void);
