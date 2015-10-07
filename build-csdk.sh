#!/bin/bash

# Use node to parse package.json and extract the upstream version from the package version. This
# assumes the package version is of the form major.minor.patch[-dev-commitid]. It grabs whatever
# follows a zero-or-more-length-series of tokens that end in a dash.
CSDK_REVISION="$(node -e '
	console.log( require( "./package.json" ).version
		.replace( /-[0-9]*$/, "" )
		.replace( /^([^-]*-)*/, "" ) );
')"

if test "x$1x" = "x--debugx"; then
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

		# iotivity wants us to clone this before it'll do anything
		git clone https://github.com/01org/tinycbor.git extlibs/tinycbor/tinycbor
		scons $SCONS_FLAGS liboctbstack libconnectivity_abstraction libcoap c_common libocsrm || { cat config.log; exit 1; }
		PREFIX="$(pwd)/../../deps/iotivity" NO_PC="true" "$(pwd)/../../install.sh" || exit 1

cd ../../ || exit 1

if test "x$1x" != "x--debugx"; then
	rm -rf depbuild || exit 1
fi
