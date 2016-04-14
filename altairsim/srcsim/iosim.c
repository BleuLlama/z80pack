/*
 * Z80SIM  -  a	Z80-CPU	simulator
 *
 * Copyright (C) 2008-2014 by Udo Munk
 *
 * This modul of the simulator contains the I/O simlation
 * for an Altair 8800 system
 *
 * History:
 * 20-OCT-08 first version finished
 * 19-JAN-14 unused I/O ports need to return 00 and not FF
 * 02-MAR-14 source cleanup and improvements
 * 14-MAR-14 added Tarbell SD FDC and printer port
 * 15-MAR-14 modified printer port for Tarbell CP/M 1.4 BIOS
 * 23-MAR-14 added 10ms timer interrupt for Kildalls timekeeper PL/M program
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include "sim.h"
#include "simglb.h"
#include "../../iodevices/io_config.h"
#include "../../iodevices/altair-88-2sio.h"
#include "../../iodevices/tarbell.h"

static int printer;		/* fd for file "printer.cpm" */

/*
 *	Forward declarations of the I/O functions
 *	for all port addresses.
 */
static BYTE io_trap_in(void), io_no_card_in(void);
static void io_trap_out(BYTE), io_no_card_out(BYTE);
static BYTE fp_in(void);
static void hwctl_out(BYTE);
static BYTE lpt_in(void);
static void lpt_out(BYTE);

/*
 *	This array contains function pointers for every
 *	input I/O port (0 - 255), to do the required I/O.
 */
static BYTE (*port_in[256]) (void) = {
	io_no_card_in,		/* port 0 */ /* no SIO card */
	io_no_card_in,		/* port	1 */ /* we got a 88-2SIO */
	io_trap_in,		/* port	2 */
	io_trap_in,		/* port	3 */
	io_trap_in,		/* port	4 */
	io_trap_in,		/* port	5 */
	io_trap_in,		/* port	6 */
	io_trap_in,		/* port	7 */
	io_trap_in,		/* port	8 */
	io_trap_in,		/* port	9 */
	io_trap_in,		/* port	10 */
	io_trap_in,		/* port	11 */
	io_trap_in,		/* port	12 */
	io_trap_in,		/* port	13 */
	io_trap_in,		/* port	14 */
	io_trap_in,		/* port	15 */
	altair_sio2_status_in,	/* port	16 */ /* SIO 1 conneted to console */
	altair_sio2_data_in,	/* port	17 */
	io_no_card_in,		/* port	18 */ /* SIO 2 not connected */
	io_no_card_in,		/* port	19 */
	io_trap_in,		/* port	20 */
	io_trap_in,		/* port	21 */
	io_trap_in,		/* port	22 */
	io_trap_in,		/* port	23 */
	io_trap_in,		/* port	24 */
	io_trap_in,		/* port	25 */
	io_trap_in,		/* port	26 */
	io_trap_in,		/* port	27 */
	io_trap_in,		/* port	28 */
	io_trap_in,		/* port	29 */
	io_trap_in,		/* port	30 */
	io_trap_in,		/* port	31 */
	io_trap_in,		/* port	32 */
	io_trap_in,		/* port	33 */
	io_trap_in,		/* port	34 */
	io_trap_in,		/* port	35 */
	io_trap_in,		/* port	36 */
	io_trap_in,		/* port	37 */
	io_trap_in,		/* port	38 */
	io_trap_in,		/* port	39 */
	io_trap_in,		/* port	40 */
	io_trap_in,		/* port	41 */
	io_trap_in,		/* port	42 */
	io_trap_in,		/* port	43 */
	io_trap_in,		/* port	44 */
	io_trap_in,		/* port	45 */
	io_trap_in,		/* port	46 */
	io_trap_in,		/* port	47 */
	io_trap_in,		/* port	48 */
	io_trap_in,		/* port	49 */
	io_trap_in,		/* port	50 */
	io_trap_in,		/* port	51 */
	io_trap_in,		/* port	52 */
	io_trap_in,		/* port	53 */
	io_trap_in,		/* port	54 */
	io_trap_in,		/* port	55 */
	io_trap_in,		/* port	56 */
	io_trap_in,		/* port	57 */
	io_trap_in,		/* port	58 */
	io_trap_in,		/* port	59 */
	io_trap_in,		/* port	60 */
	io_trap_in,		/* port	61 */
	io_trap_in,		/* port	62 */
	io_trap_in,		/* port	63 */
	io_trap_in,		/* port	64 */
	io_trap_in,		/* port	65 */
	lpt_in,			/* port	66 */ /* printer status */
	io_no_card_in,		/* port	67 */ /* printer data */
	io_trap_in,		/* port	68 */
	io_trap_in,		/* port	69 */
	io_trap_in,		/* port	70 */
	io_trap_in,		/* port	71 */
	io_trap_in,		/* port	72 */
	io_trap_in,		/* port	73 */
	io_trap_in,		/* port	74 */
	io_trap_in,		/* port	75 */
	io_trap_in,		/* port	76 */
	io_trap_in,		/* port	77 */
	io_trap_in,		/* port	78 */
	io_trap_in,		/* port	79 */
	io_trap_in,		/* port	80 */
	io_trap_in,		/* port	81 */
	io_trap_in,		/* port	82 */
	io_trap_in,		/* port	83 */
	io_trap_in,		/* port	84 */
	io_trap_in,		/* port	85 */
	io_trap_in,		/* port	86 */
	io_trap_in,		/* port	87 */
	io_trap_in,		/* port	88 */
	io_trap_in,		/* port	89 */
	io_trap_in,		/* port	90 */
	io_trap_in,		/* port	91 */
	io_trap_in,		/* port	92 */
	io_trap_in,		/* port	93 */
	io_trap_in,		/* port	94 */
	io_trap_in,		/* port	95 */
	io_trap_in,		/* port	96 */
	io_trap_in,		/* port	97 */
	io_trap_in,		/* port	98 */
	io_trap_in,		/* port	99 */
	io_trap_in,		/* port	100 */
	io_trap_in,		/* port 101 */
	io_trap_in,		/* port	102 */
	io_trap_in,		/* port	103 */
	io_trap_in,		/* port	104 */
	io_trap_in,		/* port	105 */
	io_trap_in,		/* port	106 */
	io_trap_in,		/* port	107 */
	io_trap_in,		/* port	108 */
	io_trap_in,		/* port	109 */
	io_trap_in,		/* port	110 */
	io_trap_in,		/* port	111 */
	io_trap_in,		/* port	112 */
	io_trap_in,		/* port	113 */
	io_trap_in,		/* port	114 */
	io_trap_in,		/* port	115 */
	io_trap_in,		/* port	116 */
	io_trap_in,		/* port	117 */
	io_trap_in,		/* port	118 */
	io_trap_in,		/* port	119 */
	io_trap_in,		/* port	120 */
	io_trap_in,		/* port	121 */
	io_trap_in,		/* port	122 */
	io_trap_in,		/* port	123 */
	io_trap_in,		/* port	124 */
	io_trap_in,		/* port	125 */
	io_trap_in,		/* port	126 */
	io_trap_in,		/* port	127 */
	io_no_card_in,		/* port	128 */ /* virtual hardware control */
	io_trap_in,		/* port	129 */
	io_trap_in,		/* port	130 */
	io_trap_in,		/* port	131 */
	io_trap_in,		/* port	132 */
	io_trap_in,		/* port	133 */
	io_trap_in,		/* port	134 */
	io_trap_in,		/* port	135 */
	io_trap_in,		/* port	136 */
	io_trap_in,		/* port	137 */
	io_trap_in,		/* port	138 */
	io_trap_in,		/* port	139 */
	io_trap_in,		/* port	140 */
	io_trap_in,		/* port	141 */
	io_trap_in,		/* port	142 */
	io_trap_in,		/* port	143 */
	io_trap_in,		/* port	144 */
	io_trap_in,		/* port	145 */
	io_trap_in,		/* port	146 */
	io_trap_in,		/* port	147 */
	io_trap_in,		/* port	148 */
	io_trap_in,		/* port	149 */
	io_trap_in,		/* port	150 */
	io_trap_in,		/* port	151 */
	io_trap_in,		/* port	152 */
	io_trap_in,		/* port	153 */
	io_trap_in,		/* port	154 */
	io_trap_in,		/* port	155 */
	io_trap_in,		/* port	156 */
	io_trap_in,		/* port	157 */
	io_trap_in,		/* port	158 */
	io_trap_in,		/* port	159 */
	io_trap_in,		/* port	160 */
	io_trap_in,		/* port	161 */
	io_trap_in,		/* port	162 */
	io_trap_in,		/* port	163 */
	io_trap_in,		/* port	164 */
	io_trap_in,		/* port	165 */
	io_trap_in,		/* port	166 */
	io_trap_in,		/* port	167 */
	io_trap_in,		/* port	168 */
	io_trap_in,		/* port	169 */
	io_trap_in,		/* port	170 */
	io_trap_in,		/* port	171 */
	io_trap_in,		/* port	172 */
	io_trap_in,		/* port	173 */
	io_trap_in,		/* port	174 */
	io_trap_in,		/* port	175 */
	io_trap_in,		/* port	176 */
	io_trap_in,		/* port	177 */
	io_trap_in,		/* port	178 */
	io_trap_in,		/* port	179 */
	io_trap_in,		/* port	180 */
	io_trap_in,		/* port	181 */
	io_trap_in,		/* port	182 */
	io_trap_in,		/* port	183 */
	io_trap_in,		/* port	184 */
	io_trap_in,		/* port	185 */
	io_trap_in,		/* port	186 */
	io_trap_in,		/* port	187 */
	io_trap_in,		/* port	188 */
	io_trap_in,		/* port	189 */
	io_trap_in,		/* port	190 */
	io_trap_in,		/* port	191 */
	io_trap_in,		/* port	192 */
	io_trap_in,		/* port	193 */
	io_trap_in,		/* port	194 */
	io_trap_in,		/* port	195 */
	io_trap_in,		/* port	196 */
	io_trap_in,		/* port	197 */
	io_trap_in,		/* port	198 */
	io_trap_in,		/* port	199 */
	io_trap_in,		/* port	200 */
	io_trap_in,		/* port 201 */
	io_trap_in,		/* port	202 */
	io_trap_in,		/* port	203 */
	io_trap_in,		/* port	204 */
	io_trap_in,		/* port	205 */
	io_trap_in,		/* port	206 */
	io_trap_in,		/* port	207 */
	io_trap_in,		/* port	208 */
	io_trap_in,		/* port	209 */
	io_trap_in,		/* port	210 */
	io_trap_in,		/* port	211 */
	io_trap_in,		/* port	212 */
	io_trap_in,		/* port	213 */
	io_trap_in,		/* port	214 */
	io_trap_in,		/* port	215 */
	io_trap_in,		/* port	216 */
	io_trap_in,		/* port	217 */
	io_trap_in,		/* port	218 */
	io_trap_in,		/* port	219 */
	io_trap_in,		/* port	220 */
	io_trap_in,		/* port	221 */
	io_trap_in,		/* port	222 */
	io_trap_in,		/* port	223 */
	io_trap_in,		/* port	224 */
	io_trap_in,		/* port	225 */
	io_trap_in,		/* port	226 */
	io_trap_in,		/* port	227 */
	io_trap_in,		/* port	228 */
	io_trap_in,		/* port	229 */
	io_trap_in,		/* port	230 */
	io_trap_in,		/* port	231 */
	io_trap_in,		/* port	232 */
	io_trap_in,		/* port	233 */
	io_trap_in,		/* port	234 */
	io_trap_in,		/* port	235 */
	io_trap_in,		/* port	236 */
	io_trap_in,		/* port	237 */
	io_trap_in,		/* port	238 */
	io_trap_in,		/* port	239 */
	io_trap_in,		/* port	240 */
	io_trap_in,		/* port	241 */
	io_trap_in,		/* port	242 */
	io_trap_in,		/* port	243 */
	io_trap_in,		/* port	244 */
	io_trap_in,		/* port	245 */
	io_trap_in,		/* port	246 */
	io_trap_in,		/* port	247 */
	tarbell_stat_in,	/* port	248 */ /* Tarbell 1011D status */
	tarbell_track_in,	/* port	249 */ /* Tarbell 1011D track */
	tarbell_sec_in,		/* port	250 */ /* Tarbell 1011D sector */
	tarbell_data_in,	/* port	251 */ /* Tarbell 1011D data */
	tarbell_wait_in,	/* port	252 */ /* Tarbell 1011D wait */
	io_trap_in,		/* port	253 */
	io_trap_in,		/* port	254 */
	fp_in			/* port	255 */ /* frontpanel */
};

/*
 *	This array contains function pointers for every
 *	output I/O port (0 - 255), to do the required I/O.
 */
static void (*port_out[256]) (BYTE) = {
	io_no_card_out,		/* port 0 */ /* no SIO card */
	io_no_card_out,		/* port	1 */ /* we got a 88-2SIO */
	io_trap_out,		/* port	2 */
	io_trap_out,		/* port	3 */
	io_trap_out,		/* port	4 */
	io_trap_out,		/* port	5 */
	io_trap_out,		/* port	6 */
	io_trap_out,		/* port	7 */
	io_trap_out,		/* port	8 */
	io_trap_out,		/* port	9 */
	io_trap_out,		/* port	10 */
	io_trap_out,		/* port	11 */
	io_trap_out,		/* port	12 */
	io_trap_out,		/* port	13 */
	io_trap_out,		/* port	14 */
	io_trap_out,		/* port	15 */
	altair_sio2_status_out,	/* port	16 */ /* SIO 1 conneted to console */
	altair_sio2_data_out,	/* port	17 */
	io_no_card_out,		/* port	18 */ /* SIO 2 not connected */
	io_no_card_out,		/* port	19 */
	io_trap_out,		/* port	20 */
	io_trap_out,		/* port	21 */
	io_trap_out,		/* port	22 */
	io_trap_out,		/* port	23 */
	io_trap_out,		/* port	24 */
	io_trap_out,		/* port	25 */
	io_trap_out,		/* port	26 */
	io_trap_out,		/* port	27 */
	io_trap_out,		/* port	28 */
	io_trap_out,		/* port	29 */
	io_trap_out,		/* port	30 */
	io_trap_out,		/* port	31 */
	io_trap_out,		/* port	32 */
	io_trap_out,		/* port	33 */
	io_trap_out,		/* port	34 */
	io_trap_out,		/* port	35 */
	io_trap_out,		/* port	36 */
	io_trap_out,		/* port	37 */
	io_trap_out,		/* port	38 */
	io_trap_out,		/* port	39 */
	io_trap_out,		/* port	40 */
	io_trap_out,		/* port	41 */
	io_trap_out,		/* port	42 */
	io_trap_out,		/* port	43 */
	io_trap_out,		/* port	44 */
	io_trap_out,		/* port	45 */
	io_trap_out,		/* port	46 */
	io_trap_out,		/* port	47 */
	io_trap_out,		/* port	48 */
	io_trap_out,		/* port	49 */
	io_trap_out,		/* port	50 */
	io_trap_out,		/* port	51 */
	io_trap_out,		/* port	52 */
	io_trap_out,		/* port	53 */
	io_trap_out,		/* port	54 */
	io_trap_out,		/* port	55 */
	io_trap_out,		/* port	56 */
	io_trap_out,		/* port	57 */
	io_trap_out,		/* port	58 */
	io_trap_out,		/* port	59 */
	io_trap_out,		/* port	60 */
	io_trap_out,		/* port	61 */
	io_trap_out,		/* port	62 */
	io_trap_out,		/* port	63 */
	io_trap_out,		/* port	64 */
	io_trap_out,		/* port	65 */
	io_no_card_out,		/* port	66 */ /* printer status */
	lpt_out,		/* port	67 */ /* printer data */
	io_trap_out,		/* port	68 */
	io_trap_out,		/* port	69 */
	io_trap_out,		/* port	70 */
	io_trap_out,		/* port	71 */
	io_trap_out,		/* port	72 */
	io_trap_out,		/* port	73 */
	io_trap_out,		/* port	74 */
	io_trap_out,		/* port	75 */
	io_trap_out,		/* port	76 */
	io_trap_out,		/* port	77 */
	io_trap_out,		/* port	78 */
	io_trap_out,		/* port	79 */
	io_trap_out,		/* port	80 */
	io_trap_out,		/* port	81 */
	io_trap_out,		/* port	82 */
	io_trap_out,		/* port	83 */
	io_trap_out,		/* port	84 */
	io_trap_out,		/* port	85 */
	io_trap_out,		/* port	86 */
	io_trap_out,		/* port	87 */
	io_trap_out,		/* port	88 */
	io_trap_out,		/* port	89 */
	io_trap_out,		/* port	90 */
	io_trap_out,		/* port	91 */
	io_trap_out,		/* port	92 */
	io_trap_out,		/* port	93 */
	io_trap_out,		/* port	94 */
	io_trap_out,		/* port	95 */
	io_trap_out,		/* port	96 */
	io_trap_out,		/* port	97 */
	io_trap_out,		/* port	98 */
	io_trap_out,		/* port	99 */
	io_trap_out,		/* port	100 */
	io_trap_out,		/* port 101 */
	io_trap_out,		/* port	102 */
	io_trap_out,		/* port	103 */
	io_trap_out,		/* port	104 */
	io_trap_out,		/* port	105 */
	io_trap_out,		/* port	106 */
	io_trap_out,		/* port	107 */
	io_trap_out,		/* port	108 */
	io_trap_out,		/* port	109 */
	io_trap_out,		/* port	110 */
	io_trap_out,		/* port	111 */
	io_trap_out,		/* port	112 */
	io_trap_out,		/* port	113 */
	io_trap_out,		/* port	114 */
	io_trap_out,		/* port	115 */
	io_trap_out,		/* port	116 */
	io_trap_out,		/* port	117 */
	io_trap_out,		/* port	118 */
	io_trap_out,		/* port	119 */
	io_trap_out,		/* port	120 */
	io_trap_out,		/* port	121 */
	io_trap_out,		/* port	122 */
	io_trap_out,		/* port	123 */
	io_trap_out,		/* port	124 */
	io_trap_out,		/* port	125 */
	io_trap_out,		/* port	126 */
	io_trap_out,		/* port	127 */
	hwctl_out,		/* port	128 */	/* virtual hardware control */
	io_trap_out,		/* port	129 */
	io_trap_out,		/* port	130 */
	io_trap_out,		/* port	131 */
	io_trap_out,		/* port	132 */
	io_trap_out,		/* port	133 */
	io_trap_out,		/* port	134 */
	io_trap_out,		/* port	135 */
	io_trap_out,		/* port	136 */
	io_trap_out,		/* port	137 */
	io_trap_out,		/* port	138 */
	io_trap_out,		/* port	139 */
	io_trap_out,		/* port	140 */
	io_trap_out,		/* port	141 */
	io_trap_out,		/* port	142 */
	io_trap_out,		/* port	143 */
	io_trap_out,		/* port	144 */
	io_trap_out,		/* port	145 */
	io_trap_out,		/* port	146 */
	io_trap_out,		/* port	147 */
	io_trap_out,		/* port	148 */
	io_trap_out,		/* port	149 */
	io_trap_out,		/* port	150 */
	io_trap_out,		/* port	151 */
	io_trap_out,		/* port	152 */
	io_trap_out,		/* port	153 */
	io_trap_out,		/* port	154 */
	io_trap_out,		/* port	155 */
	io_trap_out,		/* port	156 */
	io_trap_out,		/* port	157 */
	io_trap_out,		/* port	158 */
	io_trap_out,		/* port	159 */
	io_trap_out,		/* port	160 */
	io_trap_out,		/* port	161 */
	io_trap_out,		/* port	162 */
	io_trap_out,		/* port	163 */
	io_trap_out,		/* port	164 */
	io_trap_out,		/* port	165 */
	io_trap_out,		/* port	166 */
	io_trap_out,		/* port	167 */
	io_trap_out,		/* port	168 */
	io_trap_out,		/* port	169 */
	io_trap_out,		/* port	170 */
	io_trap_out,		/* port	171 */
	io_trap_out,		/* port	172 */
	io_trap_out,		/* port	173 */
	io_trap_out,		/* port	174 */
	io_trap_out,		/* port	175 */
	io_trap_out,		/* port	176 */
	io_trap_out,		/* port	177 */
	io_trap_out,		/* port	178 */
	io_trap_out,		/* port	179 */
	io_trap_out,		/* port	180 */
	io_trap_out,		/* port	181 */
	io_trap_out,		/* port	182 */
	io_trap_out,		/* port	183 */
	io_trap_out,		/* port	184 */
	io_trap_out,		/* port	185 */
	io_trap_out,		/* port	186 */
	io_trap_out,		/* port	187 */
	io_trap_out,		/* port	188 */
	io_trap_out,		/* port	189 */
	io_trap_out,		/* port	190 */
	io_trap_out,		/* port	191 */
	io_trap_out,		/* port	192 */
	io_trap_out,		/* port	193 */
	io_trap_out,		/* port	194 */
	io_trap_out,		/* port	195 */
	io_trap_out,		/* port	196 */
	io_trap_out,		/* port	197 */
	io_trap_out,		/* port	198 */
	io_trap_out,		/* port	199 */
	io_trap_out,		/* port	200 */
	io_trap_out,		/* port 201 */
	io_trap_out,		/* port	202 */
	io_trap_out,		/* port	203 */
	io_trap_out,		/* port	204 */
	io_trap_out,		/* port	205 */
	io_trap_out,		/* port	206 */
	io_trap_out,		/* port	207 */
	io_trap_out,		/* port	208 */
	io_trap_out,		/* port	209 */
	io_trap_out,		/* port	210 */
	io_trap_out,		/* port	211 */
	io_trap_out,		/* port	212 */
	io_trap_out,		/* port	213 */
	io_trap_out,		/* port	214 */
	io_trap_out,		/* port	215 */
	io_trap_out,		/* port	216 */
	io_trap_out,		/* port	217 */
	io_trap_out,		/* port	218 */
	io_trap_out,		/* port	219 */
	io_trap_out,		/* port	220 */
	io_trap_out,		/* port	221 */
	io_trap_out,		/* port	222 */
	io_trap_out,		/* port	223 */
	io_trap_out,		/* port	224 */
	io_trap_out,		/* port	225 */
	io_trap_out,		/* port	226 */
	io_trap_out,		/* port	227 */
	io_trap_out,		/* port	228 */
	io_trap_out,		/* port	229 */
	io_trap_out,		/* port	230 */
	io_trap_out,		/* port	231 */
	io_trap_out,		/* port	232 */
	io_trap_out,		/* port	233 */
	io_trap_out,		/* port	234 */
	io_trap_out,		/* port	235 */
	io_trap_out,		/* port	236 */
	io_trap_out,		/* port	237 */
	io_trap_out,		/* port	238 */
	io_trap_out,		/* port	239 */
	io_trap_out,		/* port	240 */
	io_trap_out,		/* port	241 */
	io_trap_out,		/* port	242 */
	io_trap_out,		/* port	243 */
	io_trap_out,		/* port	244 */
	io_trap_out,		/* port	245 */
	io_trap_out,		/* port	246 */
	io_trap_out,		/* port	247 */
	tarbell_cmd_out,	/* port	248 */ /* Tarbell 1011D command */
	tarbell_track_out,	/* port	249 */ /* Tarbell 1011D track */
	tarbell_sec_out,	/* port	250 */ /* Tarbell 1011D sector */
	tarbell_data_out,	/* port	251 */ /* Tarbell 1011D data */
	tarbell_ext_out,	/* port	252 */ /* Tarbell 1011D extended cmd */
	io_trap_out,		/* port	253 */
	io_trap_out,		/* port	254 */
	io_no_card_out		/* port	255 */ /* frontpanel */
};

/*
 *	This function is to initiate the I/O devices.
 *	It will be called from the CPU simulation before
 *	any operation with the CPU is possible.
 */
void init_io(void)
{
	io_config();		/* configure I/O from iodev.conf */

	/* create and open the file for line printer */
	if ((printer = creat("printer.cpm", 0644)) == -1) {
		perror("file printer.cpm");
		exit(1);
	}
}

/*
 *	This function is to stop the I/O devices. It is
 *	called from the CPU simulation on exit.
 */
void exit_io(void)
{
	close(printer);		/* close line printer file */
}

/*
 *	This is the main handler for all IN op-codes,
 *	called by the simulator. It calls the input
 *	function for port adr.
 */
BYTE io_in(BYTE adr)
{
	io_port = adr;
	return((*port_in[adr]) ());
}

/*
 *	This is the main handler for all OUT op-codes,
 *	called by the simulator. It calls the output
 *	function for port adr.
 */
void io_out(BYTE adr, BYTE data)
{
	io_port = adr;
	(*port_out[adr]) (data);
}

/*
 *	I/O input trap function
 *	This function should be added into all unused
 *	entrys of the input port array. It stops the
 *	emulation with an I/O error.
 */
static BYTE io_trap_in(void)
{
	if (i_flag) {
		cpu_error = IOTRAPIN;
		cpu_state = STOPPED;
	}
	return((BYTE) 0x00);
}

/*
 *	Same as above, but don't trap as I/O error.
 *	Used for input ports where I/O cards might be
 *	installed, but haven't.
 */
static BYTE io_no_card_in(void)
{
	return((BYTE) 0x00);
}

/*
 *      I/O output trap function
 *      This function should be added into all unused
 *      entrys of the output port array. It stops the
 *      emulation with an I/O error.
 */
static void io_trap_out(BYTE data)
{
	data++; /* to avoid compiler warning */

	if (i_flag) {
		cpu_error = IOTRAPOUT;
		cpu_state = STOPPED;
	}
}

/*
 *	Same as above, but don't trap as I/O error.
 *	Used for output ports where I/O cards might be
 *	installed, but haven't.
 */
static void io_no_card_out(BYTE data)
{
	data++; /* to avoid compiler warning */
}

/*
 *	Read input from front panel switches
 */
static BYTE fp_in(void)
{
	return(address_switch >> 8);
}

/*
 *	timer interrupt causes RST 38 in IM 0 and IM 1
 */
static void int_timer(int sig)
{
	int_type = INT_INT;
	int_code = 0xff;	/* RST 38H for IM 0 */
}

/*
 *	Virtual hardware control output.
 *	Doesn't exist in the real machine, used to shutdown
 *	and for RST 38H interrupts every 10ms.
 *
 *	bit 0 = 1	start interrupt timer
 *	bit 7 = 1       halt emulation via I/O
 */
static void hwctl_out(BYTE data)
{
	static struct itimerval tim;
	static struct sigaction newact;

	if (data & 128) {
		cpu_error = IOHALT;
		cpu_state = STOPPED;
	} else if (data & 1) {
		//printf("\r\n*** ENABLE TIMER ***\r\n");
		newact.sa_handler = int_timer;
		sigaction(SIGALRM, &newact, NULL);
		tim.it_value.tv_sec = 0;
		tim.it_value.tv_usec = 10000;
		tim.it_interval.tv_sec = 0;
		tim.it_interval.tv_usec = 10000;
		setitimer(ITIMER_REAL, &tim, NULL);
	} else if (data == 0) {
		newact.sa_handler = SIG_IGN;
		sigaction(SIGALRM, &newact, NULL);
		tim.it_value.tv_sec = 0;
		tim.it_value.tv_usec = 0;
		setitimer(ITIMER_REAL, &tim, NULL);
	}
}

/*
 *	Print data into the printer file
 */
static void lpt_out(BYTE data)
{
	if ((data != '\r') && (data != 0x00)) {
again:
		if (write(printer, (char *) &data, 1) != 1) {
			if (errno == EINTR) {
				goto again;
			} else {
				perror("write printer");
				cpu_error = IOERROR;
				cpu_state = STOPPED;
			}
		}
	}
}

/*
 *	I/O handler for line printer status in:
 *	Our printer files never is busy, so always return ready.
 */
static BYTE lpt_in(void)
{
	return((BYTE) 3);
}
