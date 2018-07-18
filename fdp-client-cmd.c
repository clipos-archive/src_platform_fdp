// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2013-2018 ANSSI. All Rights Reserved.
/*
 * Client passing file descriptor to a newly executed process.
 *
 * Author: Mickaël Salaün <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2013 SGDSN/ANSSI
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 */

#define _GNU_SOURCE

#include <clip/clip.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "fdp-common.h"
#include "fdp-client.h"

static int g_daemonized = 0;

static void
print_usage(void)
{
	fprintf(stderr, "usage: fdp-client -s <socket-path> -e <exec-path>\n");
}

static int
open_exec(char *socket_path, char *exec_path)
{
	int fd;
	int ret;
	char *fdnb;

	fd = fdp_open(socket_path, O_RDWR, 0);
	if (fd < 0) {
		printf("Failed to fdp_open");
		return -1;
	}
	ret = asprintf(&fdnb, "%d", fd);
	if (ret < 0) {
		ERROR("Failed to set envrionment");
		return -1;
	}
	setenv("FDP", fdnb, 1);

	/* Pass the FD through exec */
	execl(exec_path, exec_path, NULL);

	ERROR_ERRNO("Failed to exec %s", exec_path);
	return -1;
}

int
main(int argc, char *argv[])
{
	int ret = EXIT_SUCCESS;
	int opt = 0;
	char *socket_path = NULL;
	char *exec_path = NULL;

	while ((opt = getopt(argc, argv, "s:e:")) != -1) {
		switch (opt) {
			case 's':
				socket_path = optarg;
				break;
			case 'e':
				exec_path = optarg;
				break;
			default:
				ret = EXIT_FAILURE;
				break;
		}
		if (ret) {
			break;
		}
	}
	if (ret || !socket_path || !exec_path) {
		if (!ret) {
			ret = EXIT_FAILURE;
		}
		print_usage();
	} else {
		ret = open_exec(socket_path, exec_path);
	}

	return ret;
}
