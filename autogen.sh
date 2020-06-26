#!/bin/sh

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

(
	cd "$srcdir"
	git submodule update --init --force --remote
	aclocal -I m4
	autoheader
	automake --add-missing
	autoconf
)

(
	cd belladonna
	./autogen.sh
)

"$srcdir/configure" "$@"