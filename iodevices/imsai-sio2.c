/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2008-2014 by Udo Munk
 *
 * Partial emulation of an IMSAI SIO-2 S100 board
 *
 * History:
 * 20-OCT-08 first version finished
 * 19-JUN-14 added config parameter for droping nulls after CR/LF
 * 18-JUL-14 don't block on read from terminal
 */

#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <sys/poll.h>
#include "sim.h"
#include "simglb.h"

int sio_upper_case;
int sio_strip_parity;
int sio_drop_nulls;

/*
 * read status register
 *
 * bit 0 = 1, transmitter  ready to write character to tty
 * bit 1 = 1, character available for input from tty
 */
BYTE imsai_sio2_status_in(void)
{
	BYTE status = 0;
	struct pollfd p[1];

	p[0].fd = fileno(stdin);
	p[0].events = POLLIN | POLLOUT;
	p[0].revents = 0;
	poll(p, 1, 0);
	if (p[0].revents & POLLIN)
		status |= 2;
	if (p[0].revents & POLLOUT)
		status |= 1;

	return(status);
}

/*
 * write status register
 */
void imsai_sio2_status_out(BYTE data)
{
	data++; /* to avoid compiler warning */
}

/*
 * read data register
 *
 * can be configured to translate to upper case, most of the old software
 * written for tty's won't accept lower case characters
 */
BYTE imsai_sio2_data_in(void)
{
	BYTE data;
	struct pollfd p[1];

	p[0].fd = fileno(stdin);
	p[0].events = POLLIN;
	p[0].revents = 0;
	poll(p, 1, 0);
	if (!(p[0].revents & POLLIN))
		return(0);

	read(fileno(stdin), &data, 1);
	if (sio_upper_case)
		data = toupper(data);
	return(data);
}

/*
 * write data register
 *
 * can be configured to strip parity bit and drop nulls send after CR/LF,
 * because some old software won't
 */
void imsai_sio2_data_out(BYTE data)
{
	/* often send after CR/LF to give tty printer some time */
	if (sio_drop_nulls)
		if ((data == 127) || (data == 255) || (data == 0))
			return;

	if (sio_strip_parity)
		data &= 0x7f;

again:
	if (write(fileno(stdout), (char *) &data, 1) != 1) {
		if (errno == EINTR) {
			goto again;
		} else {
			perror("write imsai sio2 data");
			cpu_error = IOERROR;
			cpu_state = STOPPED;
		}
	}
	fflush(stdout);
	return;
}
