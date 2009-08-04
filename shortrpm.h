/* Distributed under the same license as RPM, or (at your option)
 * GNU General Public License as published by Free Software Foundation,
 * version 2 or (at your option) any later version.
 *
 * Copyright (C) 2009 Lubomir Rintel <lkundrak@v3.sk> */

#define SECTIONS(pre, post)	\
	pre "prep" post,	\
	pre "build" post,	\
	pre "install" post,	\
	pre "check" post,	\
	pre "clean" post
