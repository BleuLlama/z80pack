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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "unix_network.h"
#include "sim.h"

void telnet_negotiation(int);

/*
 * initialise a server TCP/IP socket
 */
void init_server_socket(struct connectors *p)
{
	struct sockaddr_in sin;
	int on = 1;
	int n;

	if (p->port == 0)
		return;

	if ((p->ss = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("create server socket");
		exit(1);
	}

	if (setsockopt(p->ss, SOL_SOCKET, SO_REUSEADDR, (void *) &on,
	    sizeof(on)) == -1) {
		perror("setsockopt SO_REUSEADDR on server socket");
		exit(1);
	}

	fcntl(p->ss, F_SETOWN, getpid());
	n = fcntl(p->ss, F_GETFL, 0);
	if (fcntl(p->ss, F_SETFL, n | FASYNC) == -1) {
		perror("fcntl FASYNC on server socket");
		exit(1);
	}

	memset((void *) &sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(p->port);
	if (bind(p->ss, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
		perror("bind server socket");
		exit(1);
	}

	if (listen(p->ss, 0) == -1) {
		perror("listen on server socket");
		exit(1);
	}

	printf("telnet console listening on port %d\n", p->port);
}

/*
 * SIGIO interrupt handler for server sockets
 */
void sigio_server_socket(int sig)
{
	register int i;
	struct pollfd p[NUMSOC];
	struct sockaddr_in fsin;
	socklen_t alen;
	int go_away;
	int on = 1;

	sig = sig;	/* to avoid compiler warning */

	for (i = 0; i < NUMSOC; i++) {
		p[i].fd = netcons[i].ss;
		p[i].events = POLLIN;
		p[i].revents = 0;
	}

	poll(p, NUMSOC, 0);

	for (i = 0; i < NUMSOC; i++) {
		if ((netcons[i].ss != 0) && (p[i].revents)) {
			alen = sizeof(fsin);

			if (netcons[i].ssc != 0) {
				go_away = accept(netcons[i].ss,
						 (struct sockaddr *) &fsin,
						 &alen);
				close(go_away);
				return;
			}

			if ((netcons[i].ssc = accept(netcons[i].ss,
						     (struct sockaddr *) &fsin,
						     &alen)) == -1) {
				perror("accept on server socket");
				netcons[i].ssc = 0;
			}

			if (setsockopt(netcons[i].ssc, IPPROTO_TCP, TCP_NODELAY,
			    (void *) &on, sizeof(on)) == -1) {
				perror("setsockopt TCP_NODELAY on server socket");
			}

			if (netcons[i].telnet) {
				telnet_negotiation(netcons[i].ssc);
			}
		}
	}
}

/*
 *	do the telnet option negotiation
 */
void telnet_negotiation(int fd)
{
	static char will_echo[3] = {255, 251, 1};
	static char char_mode[3] = {255, 251, 3};
	struct pollfd p[1];
	BYTE c[3];

	/* send the telnet options we need */
	write(fd, &char_mode, 3);
	write(fd, &will_echo, 3);

	/* and reject all others offered */
	p[0].fd = fd;
	p[0].events = POLLIN;
	while (1) {
		/* wait for input */
		p[0].revents = 0;
		poll(p, 1, TELNET_TIMEOUT);

		/* done if no more input */
		if (! p[0].revents)
			break;

		/* else read the option */
		read(fd, &c, 3);
		//printf("telnet: %d %d %d\r\n", c[0], c[1], c[2]);
		if (c[2] == 1 || c[2] == 3)
			continue;	/* ignore answers to our requests */
		if (c[1] == 251)	/* and reject other options */
			c[1] = 254;
		else if (c[1] == 253)
			c[1] = 252;
		write(fd, &c, 3);
	}
}
