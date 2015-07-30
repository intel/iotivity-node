#!/bin/bash

set -x

get_output_path() {
	scons -h | \
	awk \
		-v 'release=""' \
		-v 'os=""' \
		-v 'arch=""' \
		-v 'context=""' \
		'
		{
			if ( $1 == "RELEASE:" ) {
				context = "release";
			} else if ( $1 == "TARGET_OS:" ) {
				context = "os";
			} else if ( $1 == "TARGET_ARCH:" ) {
				context = "arch";
			}

			if ( $1 == "actual:" ) {
				if ( context == "release" ) {
					release = ( $2 == "True" ? "release" : "debug" );
					context = "";
				} else if ( context == "os" ) {
					os = $2;
					context = "";
				} else if ( context == "arch" ) {
					arch = $2;
					context = "";
				}
			}
		}
		END {
			print( "out/" os "/" arch "/" release );
		}
	'
}

rm -rf depbuild || exit 1
mkdir -p ./depbuild || exit 1

# Download and build iotivity from tarball
cd ./depbuild || exit 1
	wget -O iotivity.tar.gz 'https://gerrit.iotivity.org/gerrit/gitweb?p=iotivity.git;a=snapshot;h=0.9.1;sf=tgz' || exit 1
	tar xzf iotivity.tar.gz || exit 1

	cd iotivity || exit 1
		IOTIVITY_PATH="$( pwd )"
		OUTPUT_PATH="${IOTIVITY_PATH}/$( get_output_path )"
		test "x${OUTPUT_PATH}x" = "xx" && exit 1
		scons liboctbstack || { cat config.log; exit 1; }

cd ../../ || exit 1

# Run ./dist.sh
OCTBSTACK_CFLAGS="-I${IOTIVITY_PATH}/resource/csdk/stack/include" \
OCTBSTACK_LIBS="-loctbstack -L${OUTPUT_PATH} -Wl,-rpath ${OUTPUT_PATH}" \
./dist.sh || exit 1
