/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2014-2015 by Udo Munk
 *
 * Emulation of a Cromemco 4FDC/16FDC S100 board
 *
 * History:
 * 20-DEC-14 first version
 * 28-DEC-14 second version with 16FDC, CP/M 2.2 boots
 * 01-JAN-15 fixed 16FDC, machine now also boots CDOS 2.58 from 8" and 5.25"
 * 01-JAN-15 fixed frontpanel switch settings, added boot flag to fp switch
 * 12-JAN-15 implemented dummy write track, so that programs won't hang
 * 22-JAN-15 fixed buggy ID field, fake index pulses for 300/360 RPM
 * 26-JAN-15 implemented write track to format SS/SD disks properly
 * 02-FEB-15 implemented DS/DD disk formats
 * 05-FEB-15 implemented DS/SD disk formats
 * 06-FEB-15 implemented write track for all formats
 * 12-FEB-15 implemented motor control, so that a 16FDC is recogniced by CDOS
 * 20-FEB-15 bug fixes for 1.25 release
 */

/*
 * disk definitions 5.25"/8" drives, single/double density,
 * single/double sided
 */
enum Disk_type { SMALL, LARGE, UNKNOWN };
enum Disk_density { SINGLE, DOUBLE };
enum Disk_sides { ONE, TWO };

typedef struct {
	char *fn;			/* filename of disk image */
	enum Disk_type disk_t;		/* drive type 5.25" or 8" */
	enum Disk_density disk_d;	/* disk density, single or double */
	enum Disk_sides disk_s;		/* drive sides, 1 or 2 */
	int tracks;			/* # of tracks */
	int sectors;			/* # sectors on tracks > 0 side 0 */
	int sec0;			/* # sectors on track 0, side 0 */
} Diskdef;

extern BYTE fdc_flags;
extern int index_pulse;
extern int motortimer;
extern enum Disk_type dtype;

extern BYTE cromemco_fdc_status_in(void);
extern void cromemco_fdc_cmd_out(BYTE);

extern BYTE cromemco_fdc_track_in(void);
extern void cromemco_fdc_track_out(BYTE);

extern BYTE cromemco_fdc_sector_in(void);
extern void cromemco_fdc_sector_out(BYTE);

extern BYTE cromemco_fdc_data_in(void);
extern void cromemco_fdc_data_out(BYTE);

extern BYTE cromemco_fdc_diskflags_in(void);
extern void cromemco_fdc_diskctl_out(BYTE);

extern BYTE cromemco_fdc_aux_in(void);
extern void cromemco_fdc_aux_out(BYTE);
