/* Distributed under the same license as RPM, or (at your option)
 * GNU General Public License as published by Free Software Foundation,
 * version 2 or (at your option) any later version.
 *
 * Copyright (C) 2009,2016 Lubomir Rintel <lkundrak@v3.sk> */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "shortrpm.h"

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
	char **c;
	char **rpm_argv;
	int i, ia;

	static const char *sections[] = {
		SECTIONS("__spec_", "_post %{___build_post}"),
		SECTIONS("__spec_", "_pre %{___build_pre}"),
		NULL
	};

	/* Original arguments plus the --defines & trailing NULL */
	rpm_argv = malloc ((argc - 1) * sizeof(*argv) + 2 * sizeof(sections));
	if (!rpm_argv) {
		perror (argv[0]);
		return 1;
	}

	/* Copy arguments, finding out whether we're going to intercept */
	rpm_argv[0] = RPMBUILD;
	for (i = ia = 1; i < argc; i++) {
		if (strcmp (argv[i], "--short-circuit") == 0) {
			sc++;
			continue;
		} else if (strcmp (argv[i], "-bb") == 0) bb++;
		else if (strcmp (argv[i], "-tb") == 0) bb++;
		else if (strcmp (argv[i], "-ba") == 0) bb++;
		else if (strcmp (argv[i], "-ta") == 0) bb++;
		rpm_argv[ia++] = argv[i];
	}

	/* Disable section scriptlets for sections we skip*/
	if (bb && sc) {
		for (c = (char **)sections; *c; c++) {
			rpm_argv[ia++] = "--define";
			rpm_argv[ia++] = *c;
		}
	}

	rpm_argv[ia] = NULL;

	/* Pass control */
	if (bb && sc)
		putenv ("LD_PRELOAD=" SPECMANGLE);
	argv[0] = RPMBUILD;
	execv (RPMBUILD, rpm_argv);

	return 1;
}
