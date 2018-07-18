// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2013-2018 ANSSI. All Rights Reserved.
/*
 * Library function to wrap open-like calls through a FDP connection.
 *
 * Author: Mickaël Salaün <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2013 SGDSN/ANSSI
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 */

#include <clip/clip.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include "fdp-client.h"

#include "fdp-common.h"

static int g_daemonized = 0;

/* From userd:user_client.c */
static int
sock_connect(const char* sockpath)
{
	int s;
	struct sockaddr_un sau;

	sau.sun_family = AF_UNIX;
	snprintf(sau.sun_path, sizeof(sau.sun_path), "%s", sockpath);

	s = socket(PF_UNIX, SOCK_STREAM, 0);
	if (s < 0) {
		ERROR_ERRNO("socket (%s)", sockpath);
		return s;
	}
	if (connect(s, (struct sockaddr *)&sau, sizeof(struct sockaddr_un)) < 0)	{
		ERROR_ERRNO("connect %s", sockpath);
		close(s);
		return -1;
	}

	return s;
}

static int
_fdp_open(const char *pathname)
{
	int s;
	int fd;
	int ret;

	fd = -1;
	/* Fake errno if we can't get the file descriptor */
	errno = 0;

	s = sock_connect(pathname);
	if (s < 0) {
		ERROR("Failed to connect to socket");
		errno = EACCES;
		return -1;
	}
	ret = clip_recv_fd(s, &fd);
	close(s);
	if (ret) {
		ERROR("Failed to get file descriptor");
		errno = EPERM;
		return -1;
	}

	return fd;
}

/* Check if the pathname file is a socket (racy) */
static int
is_socket(const char *pathname)
{
	struct stat socket_stat;
	int ret;

	ret = stat(pathname, &socket_stat);
	if (ret) {
		return 0;
	}
	if (!S_ISSOCK(socket_stat.st_mode)) {
		return 0;
	}
	return 1;
}

/* Must be compliant with open(2) */
int
fdp_open(const char *pathname, int flags, mode_t mode)
{
	int fd;
	int fd_flags = 0;

	if (!is_socket(pathname)) {
		return open(pathname, flags, mode);
	}

	fd = _fdp_open(pathname);
	if (fd < 0) {
		return -1;
	}

	/* Set artificial O_CLOEXEC flag */
	if (flags & O_CLOEXEC) {
		fd_flags = FD_CLOEXEC;
	}
	(void)fcntl(fd, F_SETFD, fd_flags);

	/* Warning:
	 *   File access mode (O_RDONLY, O_WRONLY, O_RDWR) are ignored. Linux can
	 *   change only the O_APPEND, O_ASYNC, O_DIRECT, O_NOATIME, and O_NONBLOCK
	 *   flags (cf. fcntl(2)).
	 */
	(void)fcntl(fd, F_SETFL, flags);

	return fd;
}

/* Must be compliant with creat(2) */
inline int
fdp_creat(const char *pathname, mode_t mode)
{
	return fdp_open(pathname, O_CREAT | O_WRONLY | O_TRUNC, mode);
}

FILE *
fdp_fopen(const char *path, const char *mode)
{
	int fd;

	if (!is_socket(path)) {
		return fopen(path, mode);
	}

	/* Warning: the mode of the stream must be compatible with the fd mode */
	fd = _fdp_open(path);
	if (fd < 0) {
		return NULL;
	}

	return fdopen(fd, mode);
}

FILE *
fdp_freopen(const char *path, const char *mode, FILE *stream)
{
	if (!is_socket(path)) {
		return freopen(path, mode, stream);
	}

	(void)fclose(stream);
	return fdp_fopen(path, mode);
}
