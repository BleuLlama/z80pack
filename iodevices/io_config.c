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
 * 09-OCT-14 modified to support 2 SIO's
 */

#include <stdio.h>
#include <string.h>

#define BUFSIZE 256	/* max line length of command buffer */

extern int sio1_upper_case;	/* SIO 1 translate input to upper case */
extern int sio1_strip_parity;	/* SIO 1 strip parity from output */
extern int sio1_drop_nulls;	/* SIO 1 drop nulls after CR/LF */

extern int sio2_upper_case;	/* SIO 2 translate input to upper case */
extern int sio2_strip_parity;	/* SIO 2 strip parity from output */
extern int sio2_drop_nulls;	/* SIO 2 drop nulls after CR/LF */

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
			if (!strcmp(t1, "sio1_upper_case")) {
				switch (*t2) {
				case '0':
					sio1_upper_case = 0;
					break;
				case '1':
					sio1_upper_case = 1;
					break;
				default:
					printf("iodev.conf: illegal value for %s: %s\n", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio2_upper_case")) {
				switch (*t2) {
				case '0':
					sio2_upper_case = 0;
					break;
				case '1':
					sio2_upper_case = 1;
					break;
				default:
					printf("iodev.conf: illegal value for %s: %s\n", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio1_strip_parity")) {
				switch (*t2) {
				case '0':
					sio1_strip_parity = 0;
					break;
				case '1':
					sio1_strip_parity = 1;
					break;
				default:
					printf("iodev.conf: illegal value for %s: %s\n", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio2_strip_parity")) {
				switch (*t2) {
				case '0':
					sio2_strip_parity = 0;
					break;
				case '1':
					sio2_strip_parity = 1;
					break;
				default:
					printf("iodev.conf: illegal value for %s: %s\n", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio1_drop_nulls")) {
				switch (*t2) {
				case '0':
					sio1_drop_nulls = 0;
					break;
				case '1':
					sio1_drop_nulls = 1;
					break;
				default:
					printf("iodev.conf: illegal value for %s: %s\n", t1, t2);
					break;
				}
			} else if (!strcmp(t1, "sio2_drop_nulls")) {
				switch (*t2) {
				case '0':
					sio2_drop_nulls = 0;
					break;
				case '1':
					sio2_drop_nulls = 1;
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
