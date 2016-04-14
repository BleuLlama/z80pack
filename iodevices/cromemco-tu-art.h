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

extern BYTE cromemco_tuart_a_status_in(void);
extern void cromemco_tuart_a_baud_out(BYTE);

extern BYTE cromemco_tuart_a_data_in(void);
extern void cromemco_tuart_a_data_out(BYTE);

extern void cromemco_tuart_a_command_out(BYTE);

extern BYTE cromemco_tuart_a_interrupt_in(void);
extern void cromemco_tuart_a_interrupt_out(BYTE);

extern BYTE cromemco_tuart_a_parallel_in(void);
extern void cromemco_tuart_a_parallel_out(BYTE);

extern void cromemco_tuart_a_timer1_out(BYTE);
extern void cromemco_tuart_a_timer2_out(BYTE);
extern void cromemco_tuart_a_timer3_out(BYTE);
extern void cromemco_tuart_a_timer4_out(BYTE);
extern void cromemco_tuart_a_timer5_out(BYTE);

extern int uarta_int_mask, uarta_int, uarta_int_pending, uarta_rst7;
extern int uarta_timer1, uarta_timer2, uarta_timer3, uarta_timer4, uarta_timer5;
extern int uarta_tbe, uarta_rda;
