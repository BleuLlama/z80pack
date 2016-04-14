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
 * 20-MAR-14 ignore carriage return too, necessary for the Windows port
 * 19-JUN-14 added config parameter for droping nulls after CR/LF
 */

#include <stdio.h>
#include <string.h>

#define BUFSIZE 256	/* max line length of command buffer */

extern int sio_upper_case;	/* SIO boards translate input to upper case */
extern int sio_strip_parity;	/* SIO boards strip parity from output */
extern int sio_drop_nulls;	/* SIO boards drop nulls after CR/LF */

void io_config(void)
{
	FILE *fp;
	char buf[BUFSIZE];
	char *s, *t1, *t2;

	if ((fp = fopen("conf/iodev.conf", "r")) != NULL) {
		s = &buf[0];
		while (fgets(s, BUFSIZE, fp) != NULL) {
			if ((*s == '\n') || (*s == '\r') || (*s == '#'))
				continue;
			t1 = strtok(s, " \t");
			t2 = strtok(NULL, " \t");
			if (!strcmp(t1, "sio_upper_case")) {
				switch (*t2) {
				case '0':
					sio_upper_case = 0;
					break;
				case '1':
					sio_upper_case = 1;
					break;
				default:
					printf("iodev.conf: illegal value for %s: %s\n", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio_strip_parity")) {
				switch (*t2) {
				case '0':
					sio_strip_parity = 0;
					break;
				case '1':
					sio_strip_parity = 1;
					break;
				default:
					printf("iodev.conf: illegal value for %s: %s\n", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio_drop_nulls")) {
				switch (*t2) {
				case '0':
					sio_drop_nulls = 0;
					break;
				case '1':
					sio_drop_nulls = 1;
					break;
				default:
					printf("iodev.conf: illegal value for %s: %s\n", t1, t2);
					break;
				}
			} else {
				printf("iodev.conf unknown command: %s\n", s);
			}
		}
	}
}
