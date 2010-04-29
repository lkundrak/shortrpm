# Distributed under the same license as RPM, or (at your option)
# GNU General Public License as published by Free Software Foundation,
# version 2 or (at your option) any later version.
#
# Copyright (C) 2009,2010 Lubomir Rintel <lkundrak@v3.sk>

NAME=shortrpm
VERSION=1.3

CFLAGS ?= -Wall
PREFIX=/usr/local

CPPFLAGS = -DRPMBUILD=\"$(shell which rpmbuild)\"
CPPFLAGS ?= -DRPMBUILD=\"$(PREFIX)/bin/rpmbuild\"
CPPFLAGS += -DSPECMANGLE=\"$(PREFIX)/libexec/shortrpm/specmangle.so\"

BINS = shortrpm specmangle.so
DISTFILES = shortrpm.1 shortrpm.c shortrpm.h specmangle.c t/mangle.t \
	COPYING Makefile shortrpm.sh shortrpm.csh

override CFLAGS += -fPIC

# Build world
all: $(BINS)

# Dependencies
shortrpm.o: shortrpm.h
specmangle.o: shortrpm.h

# Clean up
clean:
	rm -rf $(BINS) *.o $(NAME)-$(VERSION){,.tar.gz}

# Install
install: $(BINS)
	install -d $(DESTDIR)$(PREFIX)/bin $(DESTDIR)$(PREFIX)/libexec/shortrpm \
		$(DESTDIR)$(PREFIX)/share/man/man1
	install -p shortrpm $(DESTDIR)$(PREFIX)/bin
	install -p specmangle.so $(DESTDIR)$(PREFIX)/libexec/shortrpm
	install -pm 0644 shortrpm.1 $(DESTDIR)$(PREFIX)/share/man/man1

# Roll distribution tarball
dist: $(NAME)-$(VERSION).tar.gz
$(NAME)-$(VERSION).tar.gz: $(DISTFILES)
	mkdir -p $(NAME)-$(VERSION)
	tar cf - $(DISTFILES) |tar xf - -C $(NAME)-$(VERSION)
	tar czf $(NAME)-$(VERSION).tar.gz $(NAME)-$(VERSION)

# Run testsuite
test: all
	./t/mangle.t

# Build DSO
.SUFFIXES: .o .so
.o.so:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -shared -ldl -o $@ $<
