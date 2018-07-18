#!/bin/sh
# SPDX-License-Identifier: LGPL-2.1-or-later
# Copyright © 2013-2018 ANSSI. All Rights Reserved.

ls -l /proc/self/fd
cat "/proc/self/fd/${FDP}"
