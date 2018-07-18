// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2013-2018 ANSSI. All Rights Reserved.
/*
 * File Descriptor Passing
 *
 * Wrapper functions using a forwarded file descriptor through a FDP connection
 * if the file path is an unix socket.
 *
 * Author: Mickaël Salaün <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2013 SGDSN/ANSSI
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 */

/* open(2) */
extern int fdp_open(const char *pathname, int flags, mode_t mode);

/* creat(2) */
extern int fdp_creat(const char *pathname, mode_t mode);

/* fopen(3) */
extern FILE *fdp_fopen(const char *path, const char *mode);

/* freopen(3) */
extern FILE *fdp_freopen(const char *path, const char *mode, FILE *stream);
