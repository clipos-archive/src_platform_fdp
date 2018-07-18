// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2013-2018 ANSSI. All Rights Reserved.
/*
 * Server opening a file and forwarding it's descriptor to each authorized clients.
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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <clip/clip.h>

#include "fdp-common.h"
#include "fdp-server.h"

static int g_daemonized = 0;
static int g_verbose = 0;

static void
print_usage(void)
{
	fprintf(stderr, "usage: fdp-server [-d] -f <file-path> -s <socket-path> -g <authorized-GID>\n");
}

static int
server_loop(struct clip_sock_t *sock_fwd)
{
	int sock;

	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		ERROR_ERRNO("signal");
		return -1;
	}

	memset(&(sock_fwd->sau), 0, sizeof(sock_fwd->sau));
	sock = clip_sock_listen(sock_fwd->path, &(sock_fwd->sau), 0);
	if (sock < 0) {
		ERROR("Failure binding socket %s", sock_fwd->name);
		/* We're not closing any already bound sockets
		 * here, but we'll die soon anyway...
		 */
		return -1;
	}
	sock_fwd->sock = sock;

	for (;;) {
		if (clip_accept_one(sock_fwd, 1, 0)) {
			ERROR("Connection failed");
		}
	}

	return -1;
}

/* From userd:cmd.c */
static int
set_nonblock(int s)
{
	int opts;
	opts = fcntl(s, F_GETFL);
	if (opts < 0) {
		ERROR_ERRNO("fcntl(F_GETFL)");
		return -1;
	}
	opts |= O_NONBLOCK;
	if (fcntl(s, F_SETFL, opts) < 0) {
		ERROR_ERRNO("fcntl(F_SETFL)");
		return -1;
	}
	return 0;
}

static int
fdp_conn_handler(int s, struct clip_sock_t *sock_fwd)
{
	int fd = -1;
	uid_t uid;
	gid_t gid;

	if (clip_getpeereid(s, &uid, &gid)) {
		ERROR("Failed to get effective GID");
		return -1;
	}
	LOG("New connection from %d:%d", uid, gid);
	if (set_nonblock(s)) {
		ERROR("Failed to set socket non-blocking");
		return -1;
	}
	if (sock_fwd->private && gid != ((struct fdp_meta *)sock_fwd->private)->gid) {
		ERROR("Effective GID don't match configuration");
		return -1;
	}
	fd = open(((struct fdp_meta *)sock_fwd->private)->file_path, O_CREAT | O_NOFOLLOW | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		ERROR_ERRNO("Failed to open or create the file");
		return -1;
	}
	if (clip_send_fd(s, fd)) {
		ERROR("Failed to send file descriptor");
		return -1;
	}
	close(fd);

	return 0;
}

int
main(int argc, char *argv[])
{
	int ret = EXIT_SUCCESS;
	int opt = 0;
	char *cwd;

	struct fdp_meta desc = {
		.gid = 0,
		.has_gid = 0,
		.file_path = NULL,
	};
	struct clip_sock_t sock_fwd = {
		.sock = -1,
		.name = "forward",
		.path = NULL,
		.handler = fdp_conn_handler,
		.private = &desc,
	};

	while ((opt = getopt(argc, argv, "f:s:g:d")) != -1) {
		switch (opt) {
			case 'f':
				desc.file_path = optarg;
				break;
			case 's':
				sock_fwd.path = optarg;
				break;
			case 'g':
				desc.gid = atoi(optarg);
				desc.has_gid = 1;
				break;
			case 'd':
				g_daemonized = 1;
				break;
			default:
				ret = EXIT_FAILURE;
				break;
		}
		if (ret) {
			break;
		}
	}
	if (ret || !desc.file_path || !sock_fwd.path || !desc.has_gid) {
		print_usage();
		return EXIT_FAILURE;
	}
	if (g_daemonized) {
		if (clip_daemonize()) {
			ERROR("Failed to daemonize");
			return EXIT_FAILURE;
		}
		openlog("fdp", LOG_CONS | LOG_PID, LOG_DAEMON);
	}

	/* Get absolute path */
	unsetenv("PWD");
	cwd = get_current_dir_name();
	LOG("Working directory: %s", cwd);
	free(cwd);
	LOG("Listening on: %s", sock_fwd.path);
	LOG("Exposing file: %s", desc.file_path);

	ret = server_loop(&sock_fwd);

	return ret;
}
