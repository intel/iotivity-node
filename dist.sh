#!/bin/bash

TESTONLY=""
DEBUG=""

while [[ $# -gt 0 ]]; do
	if test "x$1x" = "x--testonlyx" -o "x$1x" = "x-tx"; then
		TESTONLY="TRUE"
	elif test "x$1x" = "x--debugx" -o "x$1x" = "x-dx"; then
		DEBUG="--debug"
	elif test "x$1x" = "x--helpx" -o "x$1x" = "x-hx"; then
		echo "$( basename "$0" ) [--debug|-d] [--testonly|-t] [--help|-h]"
		echo "--debug or -d   : Build in debug mode"
		echo "--testonly or -t: Stop after testing"
		echo "--help or -h    : Print this message and exit"
		exit 0
	fi
	shift
done

if test "x${DEBUG}x" = "xx"; then
	MODULE_LOCATION="Release"
else
	MODULE_LOCATION="Debug"
fi

# npm install needs these variables to be in place. If they're not, let's try to establish them
# using pkg-config.
if test "x${OCTBSTACK_CFLAGS}x" = "xx"; then
	export OCTBSTACK_CFLAGS=$( pkg-config --cflags octbstack )
fi

if test "x${OCTBSTACK_LIBS}x" = "xx"; then
	export OCTBSTACK_LIBS=$( pkg-config --libs octbstack )
fi

rm -rf dist && \
mkdir -p dist/iotivity &&
npm install ${DEBUG}

if ! npm test; then
	echo "Repair build first."
	exit 1
fi

if test "x${TESTONLY}x" = "xTRUEx"; then
	exit 0
fi

npm prune --production &&
cp -a AUTHORS.txt index.js MIT-LICENSE.txt node_modules README.md dist/iotivity &&
mkdir -p dist/iotivity/build/${MODULE_LOCATION} &&
cp build/${MODULE_LOCATION}/iotivity.node dist/iotivity/build/${MODULE_LOCATION} &&
if test -d deps; then
	mkdir -p dist/iotivity/deps/iotivity/lib
	cp deps/iotivity/lib/liboctbstack.so dist/iotivity/deps/iotivity/lib
fi
cd dist &&
tar cvjf iotivity.tar.bz2 iotivity &&
cd .. &&
npm install ${DEBUG}
