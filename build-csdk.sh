#!/bin/bash

# Use node to parse package.json and extract the upstream version from the package version. This
# assumes the package version is of the form major.minor.patch[-dev-commitid]. It grabs whatever
# follows a zero-or-more-length-series of tokens that end in a dash.
CSDK_REVISION="$(node -e '
	console.log( require( "./package.json" ).version
		.replace( /-[0-9]*$/, "" )
		.replace( /^([^-]*-)*/, "" ) );
')"

DO_DEBUG=false
DO_CLEANUP=true
DO_PC=false

while [[ $# -gt 0 ]]; do
	if test "x$1x" = "x--debugx" -o "x$1x" = "x-dx"; then
		DO_DEBUG=true
		DO_CLEANUP=false
	elif test "x$1x" = "x--nocleanupx" -o "x$1x" = "x-nx"; then
		DO_CLEANUP=false
	elif test "x$1x" = "x--installpc" -o "x$1x" = "x-px"; then
		DO_PC=true
	elif test "x$1x" = "x--helpx" -o "x$1x" = "x-hx"; then
		echo "$( basename "$0" ) [options...]"
		echo ""
		echo "Possible options:"
		echo "--debug or -d     : Build in debug mode"
		echo "--nocleanup or -n : Do not clean up sources after building"
		echo "--installpc or -p : Install .pc file"
		echo "--help or -h      : Print this message and exit"
		exit 0
	fi
	shift
done


if test "x${DO_DEBUG}x" = "xtruex"; then
	SCONS_FLAGS="RELEASE=False"
fi

set -x

mkdir -p ./depbuild || exit 1

# Download and build iotivity from tarball
cd ./depbuild || exit 1
	wget -qO iotivity.tar.gz 'https://gerrit.iotivity.org/gerrit/gitweb?p=iotivity.git;a=snapshot;h='"${CSDK_REVISION}"';sf=tgz' || exit 1
	tar xzf iotivity.tar.gz || exit 1
	rm -f iotivity.tar.gz || exit 1

	# There should only be one directory inside this directory, so using the wildcard evaluates
	# exactly to it
	cd iotivity* || exit 1
		SOURCE="$(pwd)"

		# iotivity wants us to clone this before it'll do anything
		git clone https://github.com/01org/tinycbor.git extlibs/tinycbor/tinycbor
		scons $SCONS_FLAGS liboctbstack libconnectivity_abstraction libcoap c_common libocsrm || { cat config.log; exit 1; }

cd ../../ || exit 1
SOURCE="${SOURCE}" PREFIX="$(pwd)/deps/iotivity" INSTALL_PC="${DO_PC}" "$(pwd)/install.sh" || exit 1

if test "x${DO_CLEANUP}x" = "xtruex"; then
	rm -rf depbuild || exit 1
fi
