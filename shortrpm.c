/* Distributed under the same license as RPM, or (at your option)
 * GNU General Public License as published by Free Software Foundation,
 * version 2 or (at your option) any later version.
 *
 * Copyright (C) 2009 Lubomir Rintel <lkundrak@v3.sk> */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef RPMBUILD
# define RPMBUILD "/usr/bin/rpmbuild"
#endif

#ifndef SPECMANGLE
# define SPECMANGLE "/usr/lib/shortrpm/specmangle.so"
#endif

int
main (argc, argv)
	int argc;
	char *argv[];
{
	int bb = 0;
	int sc = 0;
	int i;

	/* Find out whether we're going to intercept */
	for (i = 0; i < argc; i++) {
		if (strcmp (argv[i], "-bb") == 0)
			bb++;
		if (strcmp (argv[i], "--short-circuit") == 0)
			sc++;
	}

	/* Pass control */
	if (bb && sc)
		putenv ("LD_PRELOAD=" SPECMANGLE);
	argv[0] = RPMBUILD;
	execv (RPMBUILD, argv);

	return 1;
}
