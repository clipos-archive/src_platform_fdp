// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2013-2018 ANSSI. All Rights Reserved.
/*
 * Test client for open and fopen calls.
 *
 * Author: Mickaël Salaün <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2013 SGDSN/ANSSI
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "fdp-client.h"

ssize_t
open_rw(const char *file_path, char *buf, size_t size)
{
	ssize_t len = -1;
	ssize_t len_write;
	int fd;
	const char msg[] = "hello open_rw\n";

	/* O_RDONLY flag not take into account */
	fd = fdp_open((char *)file_path, O_RDWR | O_CLOEXEC | O_APPEND, 0);
	if (fd >= 0) {
		len = read(fd, buf, size);
	}

	len_write = write(fd, msg, sizeof(msg) - 1);
	printf("len_write: %zd\n", len_write);
	close(fd);
	return len;
}

ssize_t
fopen_read(const char *file_path, char *buf, size_t size)
{
	size_t len;
	FILE *file;

	/* read-only flag not take into account */
	file = fdp_fopen(file_path, "r");
	if (!file) {
		fprintf(stderr, "Failed to open %s\n", file_path);
		return -1;
	}
	len = fread(buf, 1, size, file);
	fclose(file);
	return (ssize_t)len;
}

int
main(int argc, const char *argv[])
{
	const char *file_path = NULL;
	char buf[10];
	ssize_t len;

	if (argc < 2) {
		fprintf(stderr, "usage: test-fopen <socket-path>\n");
		return 1;
	}

	file_path = argv[1];

	len = open_rw(file_path, buf, sizeof(buf) - 1);
	if (len < 0) {
		fprintf(stderr, "open error\n");
	} else {
		buf[len] = 0;
		printf("open: %s\n", buf);
	}

	len = fopen_read(file_path, buf, sizeof(buf) - 1);
	if (len < 0) {
		fprintf(stderr, "fopen error\n");
	} else {
		buf[len] = 0;
		printf("fopen: %s\n", buf);
	}

	return 0;
}
