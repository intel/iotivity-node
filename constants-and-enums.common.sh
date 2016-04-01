# Copyright 2016 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# 	http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Code that's common between looking for enums and looking for constants
# Assumes the command line consists of OCTBSTACK_CFLAGS

find_include_paths() {
	echo "$OCTBSTACK_CFLAGS" | awk '{
		for ( idx = 1 ; idx <= NF ; idx++ ) {
			if ( substr( $idx, 1, 2 ) == "-I" ) {
				if ( length( $idx ) == 2 ) {
					print( $(idx + 1) );
				} else {
					print( substr( $idx, 3 ) );
				}
			}
		}
	}'
}

# The command line consists of the CFLAGS
OCTBSTACK_CFLAGS="$@"

INCLUDE_PATHS=$( find_include_paths )

for ONE_PATH in $INCLUDE_PATHS; do
	if test -f "${ONE_PATH}/octypes.h"; then
		OCTYPES_H="${ONE_PATH}/octypes.h"
	fi
	if test -f "${ONE_PATH}/ocpresence.h"; then
		OCPRESENCE_H="${ONE_PATH}/ocpresence.h"
	fi
	if test -f "${ONE_PATH}/ocstackconfig.h"; then
		OCSTACKCONFIG_H="${ONE_PATH}/ocstackconfig.h"
	fi
	if test -f "${ONE_PATH}/ocrandom.h"; then
		OCRANDOM_H="${ONE_PATH}/ocrandom.h"
	fi
done
