/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2014-2015 by Udo Munk
 *
 * Emulation of a Cromemco TU-ART S100 board
 *
 * History:
 *    DEC-14 first version
 *    JAN-15 better subdue of non printable characters in output
 * 02-FEB-15 implemented the timers and interrupt flag for TBE
 * 05-FEB-15 implemented interrupt flag for RDA
 * 14-FEB-15 improvements, so the the Cromix tty driver works
 */

#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <sys/poll.h>
#include "sim.h"
#include "simglb.h"

/************************/
/*	Device A	*/
/************************/

int uarta_int_mask, uarta_int, uarta_int_pending, uarta_rst7;
int uarta_timer1, uarta_timer2, uarta_timer3, uarta_timer4, uarta_timer5;
int uarta_tbe, uarta_rda;

/*
 * D7	Transmit Buffer Empty
 * D6	Read Data Available
 * D5	Interrupt Pending
 * D4	Start Bit Detected
 * D3	Full Bit Detected
 * D2	Serial Rcv
 * D1	Overrun Error
 * D0	Frame Error
 */
BYTE cromemco_tuart_a_status_in(void)
{
	BYTE status = 4;

	if (uarta_tbe)
		status |= 128;

	if (uarta_rda)
		status |= 64;

	if (uarta_int_pending)
		status |= 32;

	return(status);
}

/*
 * D7	Stop Bits	high=1 low=2
 * D6	9600		A high one of the lower seven bits selects the
 * D5	4800		corresponding baud rate. If more than one bit is high,
 * D4	2400		the highest rate selected will result. If none of the
 * D3	1200		bits are high, the serial transmitter and receiver will
 * D2	300		be disabled.
 * D1	150
 * D0	110
 */
void cromemco_tuart_a_baud_out(BYTE data)
{
	data++;	/* to avoid compiler warning */
}

BYTE cromemco_tuart_a_data_in(void)
{
	BYTE data;
	struct pollfd p[1];

	uarta_rda = 0;

	p[0].fd = fileno(stdin);
	p[0].events = POLLIN;
	p[0].revents = 0;

	poll(p, 1, 0);

	if (!(p[0].revents & POLLIN))
		return(0);

	read(fileno(stdin), &data, 1);
	return(data);
}

void cromemco_tuart_a_data_out(BYTE data)
{
	data &= 0x7f;

	uarta_tbe = 0;

	if ((data == 0x01) || (data == 0x7f))
		return;

again:
	if (write(fileno(stdout), (char *) &data, 1) != 1) {
		if (errno == EINTR) {
			goto again;
		} else {
			perror("write cromemco tu-art data");
			cpu_error = IOERROR;
			cpu_state = STOPPED;
		}
	}
	fflush(stdout);
	return;
}

/*
 * D7	Not Used
 * D6	Not Used
 * D5	Test
 * D4	High Baud
 * D3	INTA Enable
 * D3	RST7 Select
 * D1	Break
 * D0	Reset
 */
void cromemco_tuart_a_command_out(BYTE data)
{
	uarta_rst7 = (data & 4) ? 1 : 0;

	if (data & 1) {
		uarta_rda = 0;
		uarta_tbe = 1;
		uarta_timer1 = 0;
		uarta_timer2 = 0;
		uarta_timer3 = 0;
		uarta_timer4 = 0;
		uarta_timer5 = 0;
		uarta_int_pending = 0;
	}
}

/*
 * C7	Timer 1
 * CF	Timer 2
 * D7	!Sens
 * DF	Timer 3
 * E7	Receiver Data Available
 * EF	Transmitter Buffer Supply
 * F7	Timer 4
 * FF	Timer 5 or PI7
 */
BYTE cromemco_tuart_a_interrupt_in(void)
{
	return((BYTE) uarta_int);
}

/*
 * D7	Timer 5 or PI7
 * D6	Timer 4
 * D5	TBE
 * D4	RDA
 * D3	Timer 3
 * D2	!Sens
 * D1	Timer 2
 * D0	Timer 1
 */
void cromemco_tuart_a_interrupt_out(BYTE data)
{
	//printf("tu-art interrupt mask: %02x\r\n", data); fflush(stdout);
	uarta_int_mask = data;
}

/*
 *	The parallel port is used on the FDC's
 *	for auxiliary disk control/status,
 *	so don't implement something here.
 */

BYTE cromemco_tuart_a_parallel_in(void)
{
	return((BYTE) 0);
}

void cromemco_tuart_a_parallel_out(BYTE data)
{
	data++;	/* to avoid compiler warning */
}

void cromemco_tuart_a_timer1_out(BYTE data)
{
	//printf("tu-art timer 1: %d\r\n", data);
	uarta_timer1 = data;
}

void cromemco_tuart_a_timer2_out(BYTE data)
{
	//printf("tu-art timer 2: %d\r\n", data);
	uarta_timer2 = data;
}

void cromemco_tuart_a_timer3_out(BYTE data)
{
	//printf("tu-art timer 3: %d\r\n", data);
	uarta_timer3 = data;
}

void cromemco_tuart_a_timer4_out(BYTE data)
{
	//printf("tu-art timer 4: %d\r\n", data);
	uarta_timer4 = data;
}

void cromemco_tuart_a_timer5_out(BYTE data)
{
	//printf("tu-art timer 5: %d\r\n", data);
	uarta_timer5 = data;
}

/************************/
/*	Device B	*/
/************************/

/* not implemented yet */
