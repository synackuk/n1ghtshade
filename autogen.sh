#!/bin/sh

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

(
	cd "$srcdir"
	aclocal -I m4
	autoheader
	automake --add-missing
	autoconf
)

git submodule update --init --force --remote


SUBDIRS="belladonna"
for SUB in $SUBDIRS; do
    pushd $SUB
    ./autogen.sh
    popd
done

"$srcdir/configure" "$@"