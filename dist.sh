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

PACKAGE_NAME=$( node -e 'console.log( require( "./package.json" ).name );' )

DISTONLY=""
BUILDONLY=""
TESTONLY=""
DEBUG=""
REINSTONLY=""

do_npm_install() { # arguments passed through to npm install

	# npm install needs these variables to be in place. If they're not, let's try to establish them
	# using pkg-config.
	if test "x${OCTBSTACK_CFLAGS}x" = "xx"; then
		export OCTBSTACK_CFLAGS=$( pkg-config --cflags octbstack )
	fi

	if test "x${OCTBSTACK_LIBS}x" = "xx"; then
		export OCTBSTACK_LIBS=$( pkg-config --libs octbstack )
	fi

	npm install "$@"
}

buildBroken() {
	echo "Repair build first."
	exit 1
}

DO_BUILD=TRUE
DO_TEST=TRUE
DO_DIST=TRUE
DO_DEVREINST=TRUE

while [[ $# -gt 0 ]]; do
	if test "x$1x" = "x--testonlyx" -o "x$1x" = "x-tx"; then
		TESTONLY="TRUE"
	elif test "x$1x" = "x--buildonlyx" -o "x$1x" = "x-bx"; then
		BUILDONLY="TRUE"
	elif test "x$1x" = "x--distonlyx" -o "x$1x" = "x-ix"; then
		DISTONLY="TRUE"
	elif test "x$1x" = "x--noreinstallx" -o "x$1x" = "x-nx"; then
		DO_DEVREINST="FALSE"
	elif test "x$1x" = "x--reinstallonlyx" -o "x$1x" = "x-rx"; then
		REINSTONLY="TRUE"
	elif test "x$1x" = "x--debugx" -o "x$1x" = "x-dx"; then
		DEBUG="--debug"
	elif test "x$1x" = "x--helpx" -o "x$1x" = "x-hx"; then
		echo "$( basename "$0" ) [options...]"
		echo ""
		echo "Possible options:"
		echo "--debug or -d         : Build in debug mode"
		echo "--testonly or -t      : Build for testing, run tests, and exit"
		echo "--buildonly or -b     : Build and exit"
		echo "--distonly or -i      : Build distributable tree and exit"
		echo "--noreinstall or -n   : Do not reinstall dev dependencies after distribution"
		echo "--reinstallonly or -r : Reinstall regular and dev dependencies and exit"
		echo "--help or -h          : Print this message and exit"
		exit 0
	fi
	shift
done

if test "x${TESTONLY}x" = "xTRUEx"; then
	DO_BUILD=FALSE
	DO_TEST=TRUE
	DO_DIST=FALSE
	DO_DEVREINST=FALSE
fi

if test "x${BUILDONLY}x" = "xTRUEx"; then
	DO_BUILD=TRUE
	DO_TEST=FALSE
	DO_DIST=FALSE
	DO_DEVREINST=FALSE
fi

if test "x${DISTONLY}x" = "xTRUEx"; then
	DO_BUILD=FALSE
	DO_TEST=FALSE
	DO_DIST=TRUE
fi

if test "x${REINSTONLY}x" = "xTRUEx"; then
	DO_BUILD=FALSE
	DO_TEST=FALSE
	DO_DIST=FALSE
	DO_DEVREINST=TRUE
fi

if test "x${DEBUG}x" = "xx"; then
	MODULE_LOCATION="Release"
else
	MODULE_LOCATION="Debug"
fi

if test "x${DO_TEST}x" = "xTRUEx"; then
	echo "*** Performing test build/run ***"
	export TESTING=true

	if ! do_npm_install ${DEBUG}; then
		buildBroken
	fi

	if ! npm test; then
		buildBroken
	fi

	unset TESTING
fi

if test "x${DO_DIST}x" = "xTRUEx"; then
	echo "*** Performing distribution ***"

	rm -rf dist &&
	mkdir -p "dist/${PACKAGE_NAME}" &&

	( if test "x${DISTONLY}x" != "xTRUEx"; then
		if ! do_npm_install ${DEBUG}; then
			buildBroken
		fi
	fi; ) &&

	# Remove the devDependencies while keeping the production dependencies
	# https://github.com/npm/npm/issues/5590 is why prune needs to run twice
	npm prune --production &&
	npm prune --production &&
	cp -a AUTHORS.txt index.js lowlevel.js lib node_modules README.md "dist/${PACKAGE_NAME}" &&
	mkdir -p "dist/${PACKAGE_NAME}/build/${MODULE_LOCATION}" &&
	cp "build/${MODULE_LOCATION}/iotivity.node" "dist/${PACKAGE_NAME}/build/${MODULE_LOCATION}" &&
	if test -d deps; then
		mkdir -p "dist/${PACKAGE_NAME}/deps/iotivity/lib"
		cp "deps/iotivity/lib/liboctbstack.so" "dist/${PACKAGE_NAME}/deps/iotivity/lib"
	fi
	cd dist &&
	tar cvjf iotivity.tar.bz2 "${PACKAGE_NAME}" &&
	cd ..
fi

if test "x${DO_DEVREINST}x" = "xTRUEx"; then
	echo "*** Re-installing dependencies ***"

	# Restore devDependencies after having created the distribution package
	npm install --ignore-scripts
fi

if test "x${DO_BUILD}x" = "xTRUEx"; then
	echo "*** Performing build ***"
	if ! do_npm_install ${DEBUG}; then
		buildBroken
	fi
fi
