#!/bin/bash

# Update constants in src/main.cc and enums in src/enums.cc. This file uses pkgconfig to determine
# the location of octypes.h and ocstackconfig.h, but only if the environment variable
# OCTBSTACK_CFLAGS is unset.
#
# Both src/main.cc and src/enums.cc contain the comment "// The rest of this file is generated".
# This script preserves the file up to and including the comment, and discards the rest of the
# file. It then appends to src/main.cc the constant definitions from octypes.h and ocstackconfig.h
# and to src/enums.cc the enum definitions from octypes.h
#
# The script also generates the function InitEnums() and InitConstants() which the files are
# expected to export.


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

if test "x${OCTBSTACK_CFLAGS}x" = "xx"; then
	export OCTBSTACK_CFLAGS=`pkg-config --cflags octbstack`
fi

INCLUDE_PATHS=$( find_include_paths )

for ONE_PATH in $INCLUDE_PATHS; do
	if test -f $ONE_PATH/octypes.h; then
		OCTYPES_H=$ONE_PATH/octypes.h
	fi
	if test -f $ONE_PATH/octypes.h; then
		OCPRESENCE_H=$ONE_PATH/ocpresence.h
	fi
	if test -f $ONE_PATH/ocstackconfig.h; then
		OCSTACKCONFIG_H=$ONE_PATH/ocstackconfig.h
	fi
	if test -f $ONE_PATH/ocrandom.h; then
		OCRANDOM_H=$ONE_PATH/ocrandom.h
	fi
done

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
cat ${OCTYPES_H} ${OCRANDOM_H} ${OCPRESENCE_H} | \
  grep -vE '#(ifdef|define|endif)|^\s*/' | \
  grep -v '^$' | \
  awk -v PRINT=0 -v OUTPUT="" -v ENUM_LIST="" '{
    if ( $0 == "typedef enum" ) PRINT=1;
    if ( PRINT == 1 ) {
      if ( !( $1 ~ /^[{}]/ ) && $1 != "typedef" ) {
	    if ( $1 ~ /^[A-Z]/ ) {
          OUTPUT = OUTPUT "  SET_CONSTANT_NUMBER(returnValue, " $1 ");\n";
        }
      } else if ( $1 ~ /^}/ ) {
	    ENUM_NAME = $0;
	    gsub( /^} */, "", ENUM_NAME );
	    gsub( / *;.*$/, "", ENUM_NAME );
        ENUM_LIST = ENUM_LIST "  SET_ENUM(exports, " ENUM_NAME ");\n";
        print("static Local<Object> bind_" ENUM_NAME "() {\n  Local<Object> returnValue = Nan::New<Object>();\n" );
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
				printf("  SET_CONSTANT_" );
				printf( ( substr($3, 1, 1) == "\"" ) ? "STRING": "NUMBER" );
				print( "(exports, " $2 ");" );
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
parseFileForConstants ${OCSTACKCONFIG_H} >> src/constants.cc.new || \
	( rm -f src/constants.cc.new && exit 1 )

# Separate the two sections with a newline
echo '' >> src/constants.cc.new || ( rm -f src/constants.cc.new && exit 1 )

# Parse octypes.h and append to the generated file
echo '  // octypes.h: Definitions' >> src/constants.cc.new || \
	( rm -f src/constants.cc.new && exit 1 )
parseFileForConstants ${OCTYPES_H} >> src/constants.cc.new || \
	( rm -f src/constants.cc.new && exit 1 )

# Separate the two sections with a newline
echo '' >> src/constants.cc.new || ( rm -f src/constants.cc.new && exit 1 )

# Parse octypes.h and append to the generated file
echo '  // ocrandom.h: Definitions' >> src/constants.cc.new || \
	( rm -f src/constants.cc.new && exit 1 )
parseFileForConstants ${OCRANDOM_H} >> src/constants.cc.new || \
	( rm -f src/constants.cc.new && exit 1 )

# Close the function
echo '}' >> src/constants.cc.new || ( rm -f src/constants.cc.new && exit 1 )

# Replace the original file with the generated file
mv -f src/constants.cc.new src/constants.cc || ( rm -f src/constants.cc.new && exit 1 )
