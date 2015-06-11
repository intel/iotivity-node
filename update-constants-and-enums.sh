#!/bin/bash

# Update constants in src/main.cc and enums in src/enums.cc. This file uses pkgconfig to determine
# the location of octypes.h and ocstackconfig.h.
#
# Both src/main.cc and src/enums.cc contain the comment "// The rest of this file is generated".
# This script preserves the file up to and including the comment, and discards the rest of the
# file. It then appends to src/main.cc the constant definitions from octypes.h and ocstackconfig.h
# and to src/enums.cc the enum definitions from octypes.h
#
# The script also generates the function InitEnums() and InitConstants() which the files are
# expected to export.

if test "x${OCTBSTACK_CFLAGS}x" = "xx"; then
	export OCTBSTACK_CFLAGS=`pkg-config --cflags octbstack`
fi

INCLUDE_PATH=`echo "${OCTBSTACK_CFLAGS}" | sed 's/-I//g' | awk '{ print $1;}'`

# enums.cc

# Copy the boilerplate from the existing file
cat src/enums.cc | \
	awk -v PRINT=1 '{
		if ( PRINT ) print;
		if ( $0 == "// The rest of this file is generated" ) {
			PRINT=0;
			print( "" );
		}
	}' \
	> src/enums.cc.new || ( rm -f src/enums.cc.new && exit 1 )

# Parse header for enums
cat ${INCLUDE_PATH}/octypes.h | \
  grep -vE '#(ifdef|define|endif)|^\s*/' | \
  awk -v PRINT=0 -v OUTPUT="" -v ENUM_LIST="" '{
    if ( $0 == "typedef enum" ) PRINT=1;
    if ( PRINT == 1 ) {
      if ( !( $1 ~ /^[{}]/ ) && $1 != "typedef" )
        OUTPUT = OUTPUT "  SET_CONSTANT_MEMBER(returnValue, Number, " $1 ");\n";
      else if ( $1 == "}" ) {
        ENUM_NAME = substr( $2, 1, length( $2 ) - 1 );
        ENUM_LIST = ENUM_LIST "  SET_ENUM(exports, " ENUM_NAME ");\n";
        print("static Local<Object> bind_" ENUM_NAME "() {\n  Local<Object> returnValue = NanNew<Object>();\n" );
      }
      else if ( $1 != "typedef" && $1 != "{" ) {
        print;
      }
      if ( $0 ~ /;$/ ) {
        PRINT=0;
        print( OUTPUT "\n  return returnValue;\n}\n" );
        OUTPUT="";
      }
    }
  }
  END {
    print( "void InitEnums(Handle<Object> exports) {\n" ENUM_LIST "}" );
  }' | \
  sed 's/[,=]);$/);/' >> src/enums.cc.new || ( rm -f src/enums.cc.new && exit 1 )

# Replace the original file with the generated file
mv -f src/enums.cc.new src/enums.cc || ( rm -f src/enums.cc.new && exit 1 )

# src/constants.cc

# Parse header file, extracting constants
parseFileForConstants() { # $1: filename
	cat $1 | \
		grep '^#define' | \
		awk '{
			if ( NF > 2 ) {
				printf("  SET_CONSTANT_MEMBER(exports, " );
				printf( ( substr($3, 1, 1) == "\"" ) ? "String": "Number" );
				print( ", " $2 ");" );
			}
		}' | \
		sort -u
}

# Copy the boilerplate from the existing file
cat src/constants.cc | \
	awk -v PRINT=1 '{
		if ( PRINT ) print;
		if ( $0 == "// The rest of this file is generated" ) {
			PRINT=0;
			print( "" );
		}
	}' \
	> src/constants.cc.new || ( rm -f src/constants.cc.new && exit 1 )

# Add the function header
echo -e 'void InitConstants(Handle<Object> exports) {\n' >> src/constants.cc.new || \
	( rm -f src/constants.cc.new && exit 1 )

# Parse ocstackconfig.h and append to the generated file
echo '  // ocstackconfig.h: Stack configuration' >> src/constants.cc.new || \
	( rm -f src/constants.cc.new && exit 1 )
parseFileForConstants ${INCLUDE_PATH}/ocstackconfig.h >> src/constants.cc.new || \
	( rm -f src/constants.cc.new && exit 1 )

# Separate the two sections with a newline
echo '' >> src/constants.cc.new || ( rm -f src/constants.cc.new && exit 1 )

# Parse octypes.h and append to the generated file
echo '  // octypes.h: Definitions' >> src/constants.cc.new || \
	( rm -f src/constants.cc.new && exit 1 )
parseFileForConstants ${INCLUDE_PATH}/octypes.h >> src/constants.cc.new || \
	( rm -f src/constants.cc.new && exit 1 )

# Close the function
echo '}' >> src/constants.cc.new || ( rm -f src/constants.cc.new && exit 1 )

# Replace the original file with the generated file
mv -f src/constants.cc.new src/constants.cc || ( rm -f src/constants.cc.new && exit 1 )
