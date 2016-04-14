/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Common I/O devices used by various simulated machines
 *
 * Copyright (C) 2015 by Udo Munk
 *
 * This module contains functions to implement TCP/IP networking connections.
 * The sockets need to support asynchron I/O.
 *
 * History:
 * 26-MAR-15 first version finished
 */

#define TELNET_TIMEOUT 800	/* telnet negotiation timeout in milliseconds */

/* structure for TCP/IP socket connections */
struct connectors {
	int ss;		/* server socket descriptor */
	int ssc;	/* connected server socket descriptor */
	int port;	/* TCP/IP port for server socket */
	int telnet;	/* telnet protocol flag for server socket */
};

extern struct connectors netcons[];

extern void init_server_socket(struct connectors *);
void sigio_server_socket(int);
