/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2016 by Udo Munk
 * Copyright (C) 2016 by Scott Lawrence
 *
 * This modul of the simulator contains a simple terminal I/O
 * simulation as an example.
 *
 * History:
 * 28-SEP-87 Development on TARGON/35 with AT&T Unix System V.3
 * 11-JAN-89 Release 1.1
 * 08-FEB-89 Release 1.2
 * 13-MAR-89 Release 1.3
 * 09-FEB-90 Release 1.4  Ported to TARGON/31 M10/30
 * 20-DEC-90 Release 1.5  Ported to COHERENT 3.0
 * 10-JUN-92 Release 1.6  long casting problem solved with COHERENT 3.2
 *			  and some optimisation
 * 25-JUN-92 Release 1.7  comments in english and ported to COHERENT 4.0
 * 02-OCT-06 Release 1.8  modified to compile on modern POSIX OS's
 * 18-NOV-06 Release 1.9  modified to work with CP/M sources
 * 08-DEC-06 Release 1.10 modified MMU for working with CP/NET
 * 17-DEC-06 Release 1.11 TCP/IP sockets for CP/NET
 * 25-DEC-06 Release 1.12 CPU speed option
 * 19-FEB-07 Release 1.13 various improvements
 * 06-OCT-07 Release 1.14 bug fixes and improvements
 * 06-AUG-08 Release 1.15 many improvements and Windows support via Cygwin
 * 25-AUG-08 Release 1.16 console status I/O loop detection and line discipline
 * 20-OCT-08 Release 1.17 frontpanel integrated and Altair/IMSAI emulations
 * 24-JAN-14 Release 1.18 bug fixes and improvements
 * 02-MAR-14 Release 1.19 source cleanup and improvements
 * 14-MAR-14 Release 1.20 added Tarbell SD FDC and printer port to Altair
 * 29-MAR-14 Release 1.21 many improvements
 * 29-MAY-14 Release 1.22 improved networking and bugfixes
 * 04-JUN-14 Release 1.23 added 8080 emulation
 * 06-SEP-14 Release 1.24 bugfixes and improvements
 * 18-FEB-15 Release 1.25 bugfixes, improvements, added Cromemco Z-1
 * 18-APR-15 Release 1.26 bugfixes and improvements
 * 18-JAN-16 Release 1.27 bugfixes and improvements
 *
 * 05-MAY-16         1.27r RC2014 format
 */

/*
 *	I/O-handler for RC2014
 *
 *	All the unused ports are connected to an I/O-trap handler,
 *	I/O to this ports stops the simulation with an I/O error.
 */

#include <stdio.h>
#include <sys/select.h>  /* for FD_ functions */
#include <string.h> 	/* for memset(), memcpy() etc */
#include "sim.h"
#include "simglb.h"

/* ********************************************************************** */
/*
 *	Forward declarations of the I/O functions
 *	for all port addresses.
 */

static BYTE io_trap_in( void );
static void io_trap_out( BYTE );

static BYTE console_in_MC6850( void );
static void console_out_MC6850( BYTE );

static BYTE status_in_MC6850( void );
static void control_out_MC6850( BYTE );


static void SD_Init( void );

static BYTE SD_RX( void );
static void SD_TX( BYTE );

static BYTE SD_Status( void );
static void SD_Control( BYTE );


static void Bank_Init( void );

static BYTE Bank_Get( void );
static void Bank_Set( BYTE );

/* ********************************************************************** */
/*
 *	Ports and bit masks for MC6580 emulation
 */

#define kMC6850PortControl	(0x80)
	#define kPWC_Div1	0x01
	#define kPWC_Div2	0x02
		/* 0 0  / 1
		   0 1  / 16
		   1 0  / 64
		   1 1  reset
 		*/
	#define kPWC_Word1	0x04
	#define kPWC_Word2	0x08
	#define kPWC_Word3	0x10
		/* 0 0 0	7 E 2
		   0 0 1 	7 O 2
		   0 1 0	7 E 1
		   0 1 1	7 O 1
		   1 0 0	8 n 2
		   1 0 1	8 n 1
		   1 1 0	8 E 1
		   1 1 1	8 O 1
		*/
	#define kPWC_Tx1	0x20
	#define kPWC_Tx2	0x40
		/* 0 0	-RTS low, tx interrupt disabled
		   0 1  -RTS low, tx interrupt enabled
		   1 0  -RTS high, tx interrupt disabled
		   1 1  -RTS low, tx break on data, interrupt disabled
		*/
	#define kPWC_RxIrqEn	0x80

#define kMC6850PortStatus	(0x80)
	#define kPRS_RxDataReady	0x01 /* rx data is ready to be read */
	#define kPRS_TXDataEmpty	0x02 /* tx data is ready for new contents */
	#define kPRS_DCD	0x04 /* data carrier detect */
	#define kPRS_CTS	0x08 /* clear to send */
	#define kPRS_FrameErr	0x10 /* improper framing */
	#define kPRS_Overrun	0x20 /* characters lost */
	#define kPRS_ParityErr	0x40 /* parity was wrong */
	#define kPRS_IrqReq	0x80 /* irq status */

#define kMC6850PortRxData	(0x81)
#define kMC6850PortTxData	(0x81)


/* 0xDx = serial based SD card */
#define kSDPortControl	(0xD1)
#define kSDPortStatus	(0xD1)
#define kSDPortRxData	(0xD0)
#define kSDPortTxData	(0xD0)

/* 0xFx = bank swapping */
#define kBankSelect	(0xF0)

/* ********************************************************************** */
/*
 *	This array contains function pointers for every input
 *	I/O port (0 - 255), to do the required I/O.
 */
static BYTE (*port_in[256]) (void) = {
};

/*
 *	This array contains function pointers for every output
 *	I/O port (0 - 255), to do the required I/O.
 */
static void (*port_out[256]) (BYTE) = {
};


/* ********************************************************************** */

/*
 *	This function is to initiate the I/O devices.
 *	It will be called from the CPU simulation before
 *	any operation with the Z80 is possible.
 *
 *	In this sample I/O simulation we initialise all
 *	unused port with an error trap handler, so that
 *	simulation stops at I/O on the unused ports.
 *
 *	See the I/O simulation of CP/M for a more complex
 *	example.
 */
void init_io( void )
{
	register int i;

	/* default all to trap */
	for (i = 0; i <= 255; i++) {
		port_in[i] = io_trap_in;
		port_out[i] = io_trap_out;
	}

	/* set up our MC68B50 specific handlers */
	port_in[  kMC6850PortRxData  ] = console_in_MC6850;
	port_out[ kMC6850PortTxData  ] = console_out_MC6850;
	port_in[  kMC6850PortStatus  ] = status_in_MC6850;
	port_out[ kMC6850PortControl ] = control_out_MC6850;

	/* set up for the SD card serial connection */
	SD_Init();
	port_in[  kSDPortRxData  ] = SD_RX;
	port_out[ kSDPortTxData  ] = SD_TX;
	port_in[  kSDPortStatus  ] = SD_Status;
	port_out[ kSDPortControl ] = SD_Control;


	/* set up our bank switcher */
	Bank_Init();
	port_in[  kBankSelect ] = Bank_Get;
	port_out[ kBankSelect ] = Bank_Set;
}



/*
 *	This function is to stop the I/O devices. It is
 *	called from the CPU simulation on exit.
 *
 *	Nothing to do here, see the I/O simulation
 *	of CP/M for a more complex example.
 */
void exit_io( void )
{
}

/*
 *	This is the main handler for all IN op-codes,
 *	called by the simulator. It calls the input
 *	function for port adr.
 */
BYTE io_in( BYTE adr )
{
	return((*port_in[adr]) ());
}

/*
 *	This is the main handler for all OUT op-codes,
 *	called by the simulator. It calls the output
 *	function for port adr.
 */
void io_out( BYTE adr, BYTE data )
{
	(*port_out[adr]) (data);
}

/*
 *	I/O input trap function
 *	This function should be added into all unused
 *	entries of the input port array. It stops the
 *	emulation with an I/O error.
 */
static BYTE io_trap_in( void )
{
	if (i_flag) {
		cpu_error = IOTRAPIN;
		cpu_state = STOPPED;
	}
	return((BYTE) 0xff);
}

/*
 *	I/O trap function
 *	This function should be added into all unused
 *	entries of the output port array. It stops the
 *	emulation with an I/O error.
 */
static void io_trap_out( BYTE data )
{
	data++; /* to avoid compiler warning */

	if (i_flag) {
		cpu_error = IOTRAPOUT;
		cpu_state = STOPPED;
	}
}


/* ********************************************************************** */


static void SaveRam( const char * path, BYTE * mem )
{
	FILE * fp = fopen( path, "wb" );
	if( fp ) {
		fwrite( mem, 1, (64 * 1024), fp );
		fclose( fp );
	}
}

#ifndef STDIN_FILENO
#define STDIN_FILENO (0)
#endif

/* util_kbhit
 *   I done stole this from the net.
 *   A non-blocking way to tell if the user has hit a key
 */
static int util_kbhit()
{
	struct timeval tv;
	fd_set fds;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
	select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);

	return FD_ISSET(STDIN_FILENO, &fds);
}

/*
 *	I/O function port 81 read:
 *	Read next byte from stdin.
 */
static BYTE console_in_MC6850( void )
{
	int c;

	/* check if we have a key waiting so we don't block */
	if( util_kbhit() ) {
		c = fgetc( stdin );
		return( (BYTE) c );
	} 
	return (BYTE) 0xFF;
}

/*
 *	I/O function port 81 write:
 *	Write byte to stdout and flush the output.
 */
static void console_out_MC6850( BYTE data )
{
	putchar((int) data);
	fflush(stdout);
}

/*
 *	I/O function port 80 read:
 *	Get the MC6850 status register content
 */
static BYTE status_in_MC6850( void )
{
	BYTE sts = 0x00;

	if( util_kbhit() ) {
		sts |= kPRS_RxDataReady; /* key is available to read */
	}
	sts |= kPRS_TXDataEmpty;	/* we're always ready for new stuff */
	sts |= kPRS_DCD;		/* connected to a carrier */
	sts |= kPRS_CTS;		/* we're clear to send */

	return sts;
}

/*
 *	I/O function port 80 write:
 *	Set the MC6850 control register content
 */
static void control_out_MC6850( BYTE data )
{
	/* we're just going to ignore all of the control stuff for now */
	data = data;
	/* it's stuff like baud rate, start/stop/parity
 	   clearing errors and such.  we'll ignore it intentionally. */
}

/* ********************************************************************** */

extern int int_int;  /* interrupt  (0x0038)*/
extern int int_nmi;  /* non-maskable interrupt (0x0066) */
extern int int_mode; /* interrupt mode IM0, 1, 2 */
extern BYTE int_data; /* interrupt data */

void io_poll( void )
{
	/* we only care about interrupting when there's a key hit */
	if( !util_kbhit() ) return;

	switch( int_mode ) {
	case( 0 ):
		/* IM0 : data is the jump location (like RST) */
		int_data = 0xFF;	/* standard interrupt */
		int_int = 1;		/* set our interrupt flag */
		break;

	case( 1 ):
		/* IM1 : data is ignored, jump to 0x38 */
		int_int = 1;
		break;

	case( 2 ):
		/* IM2 */
		/* TBD */
		break;
	}
}

/* ********************************************************************** */

/* Serial interface to the SD card 
 *
 * We'll define a simple protocol here...
 *  *$ means "ignore everything until end of line"
 *
 * ~*$      Enter command mode
 * ~L*$     Get directory listing (Catalog)
 * ~Fname$  Filename "name"
 * ~R       Open selected file for read (will be streamed in)
 * Status byte will respond if there's more data to be read in
 */


#define kSDMode_Idle      (0)

#define kSDMode_Command   (1)

#define kSDMode_Dir       (2)

#define kSDMode_Filename  (3)
#define kSDMode_ReadFile  (4)
/* we'll handle file writing later */ 

static int mode_SD = kSDMode_Idle;
static FILE * SD_fp = NULL;
static char filename[ 32 ];
static int fnPos = 0;
static int moreToRead = 0;

/* SD_Init
 *   Initialize the SD simulator
 */
static void SD_Init( void )
{
	mode_SD = kSDMode_Idle;
	SD_fp = NULL;
	filename[0] = '\0';
	fnPos = 0;
	moreToRead = 0;
}

/* SD_RX
 *   handle the simulation of the SD module sending stuff
 *   (RX on the simulated computer)
 */
static BYTE SD_RX( void )
{
	int ch;
	switch( mode_SD ) {
	case( kSDMode_Dir ):
		/* get next byte of directory listing */
		/* dir listings are 0 terminated strings */
		/* list itself is 0 terminated (send nulls at end) */
		moreToRead = 0;
		return( 0x00 );
		break;

	case( kSDMode_ReadFile ):
		if( SD_fp ) {
			if( fread( &ch, 1, 1, SD_fp ) ) {
				moreToRead = 1;
				return ch;
			} else {
				/* end of file. close it */
				fclose( SD_fp );
			}
		}

		/* no file open, send nulls. */
		mode_SD = kSDMode_Idle;
		moreToRead = 0;
		return( 0x00 );
		break;

	case( kSDMode_Filename ):
	case( kSDMode_Command ):
	case( kSDMode_Idle ):
	default:
		// content read requested during these is ignored
		break;
	}
	return 0xff;
}


/* SD_TX
 *   handle simulation of the sd module receiving stuff
 *   (TX from the simulated computer)
 */
static void SD_TX( BYTE ch )
{
	if( mode_SD == kSDMode_ReadFile ) {
		/* do nothing... */
	}

	else if( mode_SD == kSDMode_Filename ) {
		/* store the filename until \r \n \0 or filled */
		if(    ch == 0x0a 
		    || ch == 0x0d
		    || ch == 0x00
		    || fnPos > 31 ) {
			mode_SD = kSDMode_Idle;
			printf( "EMU: SD: Filename %s\n\r", filename );
		} else {
			filename[fnPos++] = ch;
		}
		
	}

	else if( mode_SD == kSDMode_Command ) {
		switch( ch ) {
		case( 'L' ):
			mode_SD = kSDMode_Dir;
			/* open directory for read... */
			moreToRead = 0;
			break;

		case( 'F' ):
			/* reset the file name position */
			fnPos = 0;
			/* clear the string */
			do {
				int j;
				for( j=0 ; j<32 ; j++ ) {
					filename[j] = '\0';
				}
				fnPos = 0;
			} while ( 0 );

			mode_SD = kSDMode_Filename;
			break;

		case( 'R' ):
			/* open file for read */
			if( SD_fp ) fclose( SD_fp );
			SD_fp = fopen( filename, "rb" );
			if( SD_fp ) {
				
				printf( "EMU: SD: Opened %s\n\r", filename );
				moreToRead = 1;
				mode_SD = kSDMode_ReadFile;
			} else {
				printf( "EMU: SD: Failed %s\n\r", filename );
				moreToRead = 0;
				mode_SD = kSDMode_Idle;
			}
			break;

		case( 'S' ):
			SaveRam( "core.bin", ram );
			mode_SD = kSDMode_Idle;
			break;
			
		default:
			mode_SD = kSDMode_Idle;
		}
	}

	else if( ch == '~' ) {
		/* enter command mode */
		mode_SD = kSDMode_Command;
	}

	else {
		/* unknown... just return to idle. */
		mode_SD = kSDMode_Idle;
	}
	
}

static BYTE SD_Status( void )
{
	BYTE sts = 0x00;

	if( moreToRead ) {
		sts |= kPRS_RxDataReady; /* key is available to read */
	}

	sts |= kPRS_TXDataEmpty;	/* we're always ready for new stuff */
	sts |= kPRS_DCD;		/* connected to a carrier */
	sts |= kPRS_CTS;		/* we're clear to send */

	return sts;
}

static void SD_Control( BYTE data )
{
	/* ignore port settings */
	data = data;
}



/* ********************************************************************** */

/* bank switching 
 *
 * - Grant Searle's 56k RAM system has the bottom 8k as ROM, the rest is RAM
 * - Grant's and Spencer Owen's RC2014 32k RAM system right above the 8k
 * - My 64k system has RAM for the entire space
 *   - Rom can be disabled (use & 0xCF to mask out bank 3 - bank3 == 0 -> ROM ON)
 *   - 64k of space is broken down into four 8k banks
 *   - Each bank can appear in any of the four slots
 * 
 *  Address   Bank	32k	56k	64k
 * 0000-3fff   0	ROM	ROM	(ROM)	(* 64k Rom can be turned off *)
 * 0000-3fff	 	RAM	RAM	RAM
 * 4000-5fff   1	RAM	RAM	RAM
 * 6000-7fff	 	RAM	RAM	RAM
 * 8000-9fff   2  	RAM	RAM	RAM	(* Swaps with 0 *)
 * A000-Bfff	   	-	RAM	RAM
 * C000-Dfff   3	-	RAM	RAM
 * E000-ffff	 	-	RAM	RAM
 *
 *  The 64k is the version i'm emulating here
 *
 *	76 54 32 10
 *      00 00 00 00 	2 bits per slot indicates bank ram that is used there
 *      ^^------------- indicates ROM is enabled (if == 00)
 *
 * We can extend this later with higher bits for selections being in another register/port
 *
 *  On startup, it gets everything from the first 8k block, repeated.
 *  To enable full 64k, you need to write out:
 *	11 10 01 00
 *  This sets the banks to be from the proper ram sources     
 *
 *  Proper Boot sequence is probably:
 *
 *	1. Set it up so that bank 3's contents contain bank 0 space
 *	   and the rest contain business as usual.
 *		00 02 01 00	00 10 01 00   	data: 0x24
 * 	2. set our stack pointer to 0x3fff	(safe middle area)
 *	3. load in the startup rom from SD to 0xC000-ffff
 *      4. load a stub of code into stack space in ram
 * 	5. swap the RAM back down to bank 0
 *		03 02 01 00	11 10 01 00   	data: 0xE4
 * 	6. jump to code stub:
 *		A. disable ROM
 *		B. jump to 0000
 *
 *  Hard reset: (re-enable ROM, startup state)
 *	1. Make sure this code isn't running in bank 3 (C000-ffff)
 *      2. Set banking to re-enable ROM
 *     		00 10 01 00 	00 10 01 00   	data: 0x24
 *	3. jump to hard-boot stub in ROM:
 *		set bank to 0x0000
 *		jump to 0000
 *
 ********************
 * So, here's the complication:
 *   When simulating this, we have a chunk of RAM that the cpu core is using.
 *   A few things have static pointers into that, which I can't really mess 
 *   with.  I'd love to have like a 1 meg buffer, and then adjust 'ram' to 
 *   point to it, and have offsets per chunk to bank it properly.
 *   Instead...
 *   We'll use "ram" as a kind of backwards cache of the real ram.
 *   It'll all appear to be a flat chunk of ram, because it is, but in here
 *   We'll do things like swap chunks around and such so that we can simulate
 *   the effects of rom banking.  We will lose the mirroring that will happen
 *   with the real hardware, but it'll be good enough for our simualtion of it.
 */

int loadBootRom( void );
static BYTE bank_value = 0x00;

/* Bank_Init
 *   Initialize the Bank switching simulator
 */
static void Bank_Init( void )
{
	/* Initialize it all zero.
	   This maps the first 8k of ram into each 4 slots
	   This also maps the ROM into place
		since bank_value & 0xC0 == 0
	*/
	bank_value = 0x00;
}

static BYTE Bank_Get( void )
{
	return bank_value;
}

/* this is the backup buffer we use while copying ram around/doing swaps */
BYTE ramBackup[ 64 * 1024 ];

int BankToAddr( int bnk )
{
	int addr = 0;

	switch( bnk ) {
	case( 0 ): addr = (bank_value & 0x03); break;
	case( 1 ): addr = ((bank_value >> 2) & 0x03); break;
	case( 2 ): addr = ((bank_value >> 4) & 0x03); break;
	case( 3 ): addr = ((bank_value >> 6) & 0x03); break;
	default: break;
	}

	return addr * 0x4000;
}


/* RamToBackup
 *  copy the mapped ram in the cpu space to the backup
 *  in the appropriate space
 */
static void Bank_RamToBackup()
{
	int j;
	for( j=0 ; j<4 ; j++ ) {
		BYTE * src = ram + (0x4000*j);
		BYTE * dst = ramBackup + BankToAddr( j );
		memcpy( dst, src, 0x4000 );
	}
}

/* BackupToRam
*   Copy the backup into the cpu space in the banked locations
*/
static void Bank_BackupToRam()
{
	int j;
	for( j=0 ; j<4 ; j++ ) {
		BYTE * src = ramBackup + (0x4000*j);
		BYTE * dst = ram + BankToAddr( j );
		memcpy( dst, src, 0x4000 );
	}
}

static void Bank_Set( BYTE data )
{
	printf( "EMU: Bank config: %02x\n\r", data );
	/* We're only using this for our bank switching which currently
           is just bank swapping. So we're going to do some crazy stuff
 	   in here which does not function like the actual hardware,
	   but rather performs an analogous function with some known
	   side effects.
	   Basically, we will perform the required move, but anything
	   "stored" between calls will be lost.
	*/

	/* 1. copy the ram out of CPU space into our backup, remapped */
	Bank_RamToBackup();

	/* 2. Clear the cpu ram */
	memset( ram, 0x00, 1024 * 64 );

	/* 3. Reconfigure the bank */
	bank_value = data;

	/* 4. Copy the ram back from the backup */
	Bank_BackupToRam();

	/* 5. Copy the ROM if applicable */
	if( (bank_value & 0xC0) == 0x00 ) {
		/* sentinel to enable the ROM */
		printf( "EMU: Bank 0: ROM\n\r" );
		memset( ram, 0x00, 1024 * 8 );
		(void)loadBootRom();
	}
}

