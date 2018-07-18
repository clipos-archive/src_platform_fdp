# SPDX-License-Identifier: LGPL-2.1-or-later
# Copyright © 2013-2018 ANSSI. All Rights Reserved.
CC = gcc
LDLIBS = -lclip
LDLIBS_CLIENT = -lfdp-client $(LDLIBS)

CFLAGS += -fPIC -Wall -Wextra -Werror
LDFLAGS_SHARED = -shared $(LDFLAGS)
LDFLAGS_BIN = -pie -L. $(LDFLAGS)

PREFIX ?= /usr
CLIP_CTX ?= none

SOBASE = libfdp-client.so
SONAME = $(SOBASE).1
SOVERSION = $(SOBASE).1.0.0
BIN_FILES = fdp-server fdp-client
LIB_FILES = $(SOVERSION)
LIB_LINKS = $(SOBASE) $(SONAME)
INC_FILES = fdp-client.h

.PHONY: all clean mrproper install

all: $(SOBASE) $(BIN_FILES) test-fopen

clean:
	rm fdp-server.o fdp-client.o fdp-client-cmd.o test-fopen.o || true

mrproper: clean
	rm $(BIN_FILES) $(LIB_FILES) $(LIB_LINKS) test-fopen || true

install: $(SOBASE) $(BIN_FILES)
	for bin in $(BIN_FILES); do \
		install -D $${bin} $(DESTDIR)/$(PREFIX)/bin/$${bin}; \
	done
	for lib in $(LIB_FILES); do \
		install -D $${lib} $(DESTDIR)/$(PREFIX)/lib/$${lib}; \
	done
	for slib in $(LIB_LINKS); do \
		cp -R $${slib} $(DESTDIR)/$(PREFIX)/lib/$${slib}; \
	done
	for inc in $(INC_FILES); do \
		install -m 0644 -D $${inc} $(DESTDIR)/$(PREFIX)/include/$${inc}; \
	done
	if [ '$(CLIP_CTX)' = 'rm' ]; then \
		install -m 0755 -D ssm-db-serve $(DESTDIR)/$(PREFIX)/bin/clip-user-data-update-scripts/ssm-db-serve; \
	elif [ '$(CLIP_CTX)' = 'clip' ]; then \
		install -m 0755 -D ssm-key-serve-user $(DESTDIR)/$(PREFIX)/bin/ssm-key-serve-user; \
		install -m 0755 -D ssm-key-serve-socle $(DESTDIR)/$(PREFIX)/bin/ssm-key-serve-socle; \
	fi

uninstall: $(SOBASE) $(BIN_FILES)
	for bin in $(BIN_FILES); do \
		rm $(DESTDIR)/$(PREFIX)/bin/$${bin}; \
	done
	for lib in $(LIB_FILES); do \
		rm $(DESTDIR)/$(PREFIX)/lib/$${lib}; \
	done
	for slib in $(LIB_LINKS); do \
		rm $(DESTDIR)/$(PREFIX)/lib/$${slib}; \
	done
	for inc in $(INC_FILES); do \
		rm $(DESTDIR)/$(PREFIX)/include/$${inc}; \
	done

fdp-server: fdp-server.o
	$(CC) -o $@ $(LDLIBS) $(LDFLAGS_BIN) $^

$(SOBASE): fdp-client.o
	$(CC) -o $(SOVERSION) -Wl,-soname,$(SONAME) $(LDLIBS) $(LDFLAGS_SHARED) $^
	ln -sf $(SOVERSION) $(SONAME)
	ln -sf $(SOVERSION) $@

fdp-client: $(SOBASE) fdp-client-cmd.o
	$(CC) -o $@ $(LDLIBS_CLIENT) $(LDFLAGS_BIN) $^

test-fopen: $(SOBASE) test-fopen.o
	$(CC) -o $@ $(LDLIBS_CLIENT) $(LDFLAGS_BIN) $^

%.o: %.c %.h fdp-common.h
	$(CC) -c -o $@ $(CFLAGS) $<

%.o: %.c fdp-common.h
	$(CC) -c -o $@ $(CFLAGS) $<
