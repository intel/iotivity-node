#!/bin/bash

# npm install needs these variables to be in place. If they're not, let's try to establish them
# using pkg-config.
if test "x${OCTBSTACK_CFLAGS}x" = "xx"; then
	export OCTBSTACK_CFLAGS=`pkg-config --cflags octbstack`
fi

if test "x${OCTBSTACK_LIBS}x" = "xx"; then
	export OCTBSTACK_LIBS=`pkg-config --libs octbstack`
fi

rm -rf dist && \
mkdir -p dist/iotivity &&
npm install

if ! npm test; then
	echo "Repair build first."
	exit 1
fi

npm prune --production &&
cp -a AUTHORS.txt index.js MIT-LICENSE.txt node_modules README.md dist/iotivity &&
mkdir -p dist/iotivity/build/Release &&
cp build/Release/iotivity.node dist/iotivity/build/Release &&
cd dist &&
tar cvjf iotivity.tar.bz2 iotivity &&
cd .. &&
npm install
