/*
 * Copyright 2006 - 2010 Heinz Mauelshagen, Red Hat GmbH
 *
 * Simple write_pattern/check_pattern program to write varying/static
 * patterns to a file/device and check they can be read back correctly.
 *
 * Create 2 links write_pattern/read_pattern to the created executable.
 *
 */

#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define	BSIZE	1024

int main(int argc, char **argv)
{
	int fd, r = 0, repeat = 0, wr;
	char *cmd = basename(argv[0]), *dev, buf[BSIZE], rbuf[BSIZE];
	size_t i, sz, sectors;

	if (argc < 3 || argc > 4) {
		fprintf(stderr, "%s dev_path #sectors [repeat]\n", cmd);
		return 1;
	}

	dev = argv[1];
	sectors = atoi(argv[2]);
	if (!sectors) {
		fprintf(stderr, "%s - #sectors have to be > 0\n", cmd);
		return 2;
	}

	wr = !strncmp(cmd, "wr", 2);

	if (argc > 3) {
		if (strcmp(argv[3], "repeat")) {
			fprintf(stderr, "%s - invalid repeat argument\n", cmd);
			return 3;
		}

		repeat = 1;
	}

	if ((fd = open(dev, wr ? O_WRONLY : O_RDONLY)) == -1) {
		fprintf(stderr, "%s - failed to open %s\n", cmd, dev);
		return 4;
	}

	for (i = 0; i < sectors; i++) {
		memset(buf, ' ', sizeof(buf));
		sz = 0;
		do {
			sz += sprintf(buf + sz, "%lu,", repeat ? 4711 : i);
		} while (sz < sizeof(buf));

		if (wr) {
			if (write(fd, buf, sizeof(buf)) != sizeof(buf)) {
				fprintf(stderr, "%s - write sector=%lu failed\n",
					cmd, i);
				r = 6;
				break;
			}
		} else {
			memset(rbuf, 0, sizeof(rbuf));
			if (read(fd, rbuf, sizeof(buf)) != sizeof(buf)) {
				fprintf(stderr, "%s - read sector=%lu failed\n",
					cmd, i);
				r = 6;
				break;
			}

			if (memcmp(buf, rbuf, sizeof(buf))) {
				printf(" %lu!", i);
				r = 7;
				break;
			}
		}

		if (!((i + 1) % 10000)) {
			putchar('.');
			fflush(stdout);
		}
	}

	putchar('\n');
	close(fd);
	return r;
}
