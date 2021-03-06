/* Distributed under the same license as RPM, or (at your option)
 * GNU General Public License as published by Free Software Foundation,
 * version 2 or (at your option) any later version.
 *
 * Copyright (C) 2009,2010 Lubomir Rintel <lkundrak@v3.sk> */

#define _GNU_SOURCE 1

#include <ctype.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "shortrpm.h"

/* Filename matches *.spec */
static int
is_spec (filename)
	const char *filename;
{
	if (strlen (filename) < 5)
		return 0;
	return strcmp (filename + strlen (filename) - 5, ".spec") == 0;
}

/* glibc open */
static int
real_open (filename, flags, mode)
	const char *filename;
	int flags;
	mode_t mode;
{
	int (*libc_open)(const char *, int, mode_t);
	libc_open = dlsym (RTLD_NEXT, "open64");
	if (libc_open == NULL)
		libc_open = dlsym (RTLD_NEXT, "open");
	if (libc_open == NULL) {
		errno = EFAULT;
		return -1;
	}
	return libc_open (filename, flags, mode);
}

/* glibc fopen */
FILE *
real_fopen (filename, modes)
	const char *filename;
	const char *modes;
{
	FILE *(*libc_fopen)(const char *, const char *);
	libc_fopen = dlsym (RTLD_NEXT, "fopen64");
	if (libc_fopen == NULL)
		libc_fopen = dlsym (RTLD_NEXT, "fopen");
	if (libc_fopen == NULL) {
		errno = EFAULT;
		return NULL;
	}
	return libc_fopen (filename, modes);
}

/* Matches "[whitespace]*%section" */
static int
is_section (line)
	char *line;
{
	char *p;
	static const char *sections[] =  { SECTIONS("%", ""), NULL };
	const char **section;

	p = line;
	while (isspace (*p))
		p++;

	for (section = sections; *section; section++) {
		char separator;

		if (strstr (p, *section) != p)
			continue;

		separator = *(p + strlen (*section));
		if (isspace (separator) || !separator)
			return 1;
	}

	return 0;
}

/* Given the descriptor of the original file,
 * write out the faked one */
static int
fake_spec (fake_fd, real_fd)
	int fake_fd, real_fd;
{
	FILE *fake, *real;
	char *line;
	int rec;

	/* Let the C library take care of buffering */
	real = fdopen (real_fd, "r");
	if (real == NULL)
		return 1;
	fake = fdopen (fake_fd, "w");
	if (fake == NULL)
		return 1;

	while ((rec = fscanf (real, "%a[^\n]", &line)) != -1) {
		if (rec) {
			/* Line contents */
			fputs (line, fake);
			if (is_section (line))
				fputs ("\nexit 0", fake);
			free (line);
		}

		/* Trailing breaks */
		if (fscanf (real, "%a[\n]", &line) != 1)
			break;
		fputs (line, fake);
		free (line);
	}

	return 0;
}

/* Return a descriptor of pipe from which the
 * faked SPEC file is to be read from */
static int
fake_open (filename, flags)
	const char *filename;
	int flags;
{
	int fds[2];
	int real_fd;
	int saved_errno;

	if (pipe (fds) == -1) {
		saved_errno = errno;
		goto pipe_fail;
	}

	real_fd = real_open (filename, flags, 0);
	if (real_fd == -1) {
		saved_errno = errno;
		goto open_fail;
	}

	switch (fork ()) {
	case -1:
		saved_errno = errno;
		goto fork_fail;
	case 0:
		close (fds[0]);
		/* Deamonize so that we are not waited for */
		switch (fork ()) {
		case 0:
			exit (-fake_spec (fds[1], real_fd));
		case -1:
			exit (1);
		default:
			exit (0);
		}
	default:
		close (real_fd);
		close (fds[1]);
		return fds[0];
	}

fork_fail:
	close (real_fd);
open_fail:
	close (fds[0]);
	close (fds[1]);
pipe_fail:
	errno = saved_errno;
	return -1;
}

/* Override standard open() */
int
open (const char *filename, int flags, ...)
{
	mode_t mode = 0;

	if (flags & O_CREAT) {
		va_list args;
		va_start (args, flags);
		mode = va_arg (args, int);
		va_end (args);
	}

	if ((flags == 0 || flags & O_RDONLY) && is_spec (filename)) {
		return fake_open (filename, flags);
	}

	return real_open (filename, flags, mode);
}

/* Override standard fopen() */
FILE *
fopen (filename, modes)
	const char *filename;
	const char *modes;
{
	if (is_spec (filename) && strcmp (modes, "r") == 0) {
		int fd = fake_open (filename, O_RDONLY);
		return fdopen (fd, modes);
	}

	return real_fopen (filename, modes);
}

/* Largefile open() */
int
open64 (const char *, int, ...) __attribute__ ((alias ("open")));

/* Largefile fopen() */
FILE *
fopen64 (const char *, const char *) __attribute__ ((alias ("fopen")));
