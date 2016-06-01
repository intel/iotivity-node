#!/bin/bash

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

# Update constants in generated/constants.cc. The command line arguments for this script are used to
# construct the value of the OCTBSTACK_CFLAGS variable, which, in turn, is used for determining
# where the include files are located.
#
# generated/constants.cc.in contains the comment "// The rest of this file is generated". This script
# preserves the file up to and including the comment, and appends generated contents after that. It
# then appends to generated/constants.cc the constant definitions from octypes.h and ocstackconfig.h.
#
# The script also generates the function InitConstants() which the file is expected to export.

. ./constants-and-enums.common.sh

# generated/constants.cc

# Parse header file, extracting constants
parseFileForConstants() { # $1: filename
	grep '^#define' < "$1" | \
	awk '{
		if ( NF > 2 && $2 !~ /[()]/ ) {
			print( "SET_CONSTANT_" ( ( substr($3, 1, 1) == "\"" ) ? "STRING": "NUMBER" ) " " $2 );
		}
	}' | \
	sort -u | \
	awk '{
		print( "#ifdef " $2 );
		print( "  " $1 "(exports, " $2 ");" );
		print( "#endif /* def " $2 "*/" );
	}'
}

mkdir -p generated

# Copy the boilerplate from the starter file
cat src/constants.cc.in > generated/constants.cc.new || ( rm -f generated/constants.cc.new && exit 1 )

# Add the function header
echo 'void InitConstants(Handle<Object> exports) {' >> generated/constants.cc.new || \
	( rm -f generated/constants.cc.new && exit 1 )

# Parse ocstackconfig.h and append to the generated file
echo '  // ocstackconfig.h: Stack configuration' >> generated/constants.cc.new || \
	( rm -f generated/constants.cc.new && exit 1 )
parseFileForConstants "${OCSTACKCONFIG_H}" >> generated/constants.cc.new || \
	( rm -f generated/constants.cc.new && exit 1 )

# Separate the two sections with a newline
echo '' >> generated/constants.cc.new || ( rm -f generated/constants.cc.new && exit 1 )

# Parse octypes.h and append to the generated file
echo '  // octypes.h: Definitions' >> generated/constants.cc.new || \
	( rm -f generated/constants.cc.new && exit 1 )
parseFileForConstants "${OCTYPES_H}" >> generated/constants.cc.new || \
	( rm -f generated/constants.cc.new && exit 1 )

# Separate the two sections with a newline
echo '' >> generated/constants.cc.new || ( rm -f generated/constants.cc.new && exit 1 )

# Parse octypes.h and append to the generated file
echo '  // ocrandom.h: Definitions' >> generated/constants.cc.new || \
	( rm -f generated/constants.cc.new && exit 1 )
parseFileForConstants "${OCRANDOM_H}" >> generated/constants.cc.new || \
	( rm -f generated/constants.cc.new && exit 1 )

# Close the function
echo '}' >> generated/constants.cc.new || ( rm -f generated/constants.cc.new && exit 1 )

# Replace the original file with the generated file
mv -f generated/constants.cc.new generated/constants.cc || ( rm -f generated/constants.cc.new && exit 1 )
