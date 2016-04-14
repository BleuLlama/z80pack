/*
 * converts binary paper tape files to raw binary by removing
 * the leading 0's
 *
 * Copyright (C) 2015 by Udo Munk
 *
 * History:
 * 04-APR-15 first version
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
	register int n;
	int fdin, fdout;
	int flag = 0;
	char c;
	char *pn = basename(argv[0]);

	if (argc != 3) {
		printf("usage: %s infile outfile\n", pn);
		return(1);
	}

	if ((fdin = open(argv[1], O_RDONLY)) == -1) {
		perror(argv[1]);
		return(1);
	}

	if ((fdout = open(argv[2], O_WRONLY|O_CREAT, 0644)) == -1) {
		perror(argv[2]);
		return(1);
	}

	while ((n = read(fdin, &c, 1)) == 1) {
		if (flag == 0) {
			if (c == 0)
				continue;
			else
				flag++;
		}
		write(fdout, &c, 1);
	}

	close(fdin);
	close(fdout);

	return(0);
}
