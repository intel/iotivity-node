#!/bin/bash

# Update enums in generated/enums.cc. The command line arguments for this script are used to construct
# the value of the OCTBSTACK_CFLAGS variable, which, in turn, is used for determining where the
# include files are located.
#
# generated/enums.cc contains the comment "// The rest of this file is generated". This script
# preserves the file up to and including the comment, and discards the rest of the file. It then
# appends to generated/enums.cc the enum definitions from octypes.h
#
# The script also generates the function InitEnums() which the file is expected to export.

. ./constants-and-enums.common.sh

# enums.cc

mkdir -p generated

# Copy the boilerplate from the existing file
cat src/enums.cc.in > generated/enums.cc.new || ( rm -f generated/enums.cc.new && exit 1 )

# Parse header for enums
cat "${OCTYPES_H}" "${OCRANDOM_H}" "${OCPRESENCE_H}" | \
  grep -v '^$' | \
  awk -v PRINT=0 -v OUTPUT="" -v ENUM_LIST="" '{
    if ( $0 == "typedef enum" ) PRINT=1;
    if ( PRINT == 1 ) {
      if ( !( $1 ~ /^[{}]/ ) && $1 != "typedef" ) {
	    if ( $1 ~ /^[A-Z]/ ) {
          OUTPUT = OUTPUT "  SET_CONSTANT_NUMBER(returnValue, " $1 ");\n";
        } else if ( $1 ~ /^#(if|endif)/ ) {
          OUTPUT = OUTPUT $0 "\n";
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
  sed 's/[,=]);$/);/' >> generated/enums.cc.new || ( rm -f generated/enums.cc.new && exit 1 )

# Replace the original file with the generated file
mv -f generated/enums.cc.new generated/enums.cc || ( rm -f generated/enums.cc.new && exit 1 )
