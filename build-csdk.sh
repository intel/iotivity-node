#!/bin/bash

# Copyright 2016 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Use node to parse package.json and extract the upstream version from the package version. This
# assumes the package version is of the form major.minor.patch[-dev-commitid]. It grabs whatever
# follows a zero-or-more-length-series of tokens that end in a dash.
if test "x${CSDK_REVISION}x" = "xx"; then
	CSDK_REVISION="$(node -p '
		require( "./package.json" ).version
			.replace( /-[0-9]*$/, "" )
			.replace(  /^([^-]*-)pre-(.*$)/, "$2" )
	')"

	# The second .replace() will not match anything if it's not a commitid-version, so we will be
	# left with the exact upstream version
fi

DO_BUILD=true
DO_DEBUG=false
DO_CLEANUP=true
DO_PC=false

while test $# -gt 0; do
	if test "x$1x" = "x--debugx" -o "x$1x" = "x-dx"; then
		DO_DEBUG=true
		DO_CLEANUP=false
	elif test "x$1x" = "x--nocleanupx" -o "x$1x" = "x-nx"; then
		DO_CLEANUP=false
	elif test "x$1x" = "x--installpc" -o "x$1x" = "x-px"; then
		DO_PC=true
	elif test "x$1x" = "x--install-only" -o "x$1x" = "x-ix"; then
		DO_BUILD=false
	elif test "x$1x" = "x--helpx" -o "x$1x" = "x-hx"; then
		echo "$( basename "$0" ) [options...]"
		echo ""
		echo "Possible options:"
		echo "--debug or -d        : Build in debug mode"
		echo "--help or -h         : Print this message and exit"
		echo "--install-only or -i : Do not build, just install"
		echo "--installpc or -p    : Install .pc file"
		echo "--nocleanup or -n    : Do not clean up sources after building"
		exit 0
	fi
	shift
done


if test "x${DO_DEBUG}x" = "xtruex"; then
	SCONS_FLAGS="RELEASE=False"
fi

set -x

if test "x${DO_BUILD}x" = "xtruex"; then
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

			ls ../../patches/*.patch | sort | while read patch; do
				patch -p1 < "${patch}"
			done

			scons $SCONS_FLAGS logger octbstack connectivity_abstraction coap c_common ocsrm routingmanager || { cat config.log; exit 1; }

	cd ../../ || exit 1
else
	SOURCE=$(ls -d ./depbuild/iotivity* | while read; do if test -d "${REPLY}"; then echo "${REPLY}"; break; fi; done )
fi

SOURCE="${SOURCE}" PREFIX="$(pwd)/deps/iotivity" INSTALL_PC="${DO_PC}" "$(pwd)/install.sh" || exit 1

if test "x${DO_CLEANUP}x" = "xtruex"; then
	rm -rf depbuild || exit 1
fi
