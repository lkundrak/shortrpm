#!/bin/bash

# Distributed under the same license as RPM, or (at your option)
# GNU General Public License as published by Free Software Foundation,
# version 2 or (at your option) any later version.
#
# Copyright (C) 2009 Lubomir Rintel <lkundrak@v3.sk>

trap 'rm -f check.spec check.nospec' EXIT

RETVAL=0
check_status ()
{
	if [ $? = 0 ]
	then
		echo "ok:" "$@"
	else
		echo "not ok:" "$@"
		RETVAL=$(( $RETVAL + 1 ))
	fi
}

VANILLA="

%prep
% build
foo % install
%check bar
  %prep

  %chuck 
  %build zzz
%%install
%  %install
%build"

PROCESSED="

%prep
exit 0
% build
foo % install
%check bar
exit 0
  %prep
exit 0

  %chuck 
  %build zzz
exit 0
%%install
%  %install
%build
exit 0"

echo 1..2

echo -n "$VANILLA" >check.spec
diff -u <(echo -n "$PROCESSED") <(LD_PRELOAD=$PWD/specmangle.so cat check.spec)
check_status "spec file was mangled"

echo -n "$VANILLA" >check.nospec
diff -u <(echo -n "$VANILLA") <(LD_PRELOAD=$PWD/specmangle.so cat check.nospec)
check_status "nospec file was not mangled"

exit $RETVAL
