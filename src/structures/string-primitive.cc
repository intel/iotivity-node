#include "string-primitive.h"
#include "../common.h"

extern "C" {
#include <string.h>
}

using namespace v8;

bool c_StringNew( Local<String> jsString, char **p_string ) {
	size_t length = strlen( ( const char * )*( String::Utf8Value( jsString ) ) );
	char *string = ( char * )malloc( length + 1 );
	if ( !string ) {
		NanThrowError( "Failed to allocate memory for C string" );
		return false;
	}
	string[ length ] = 0;
	strcpy( string, ( const char * )*( String::Utf8Value( jsString ) ) );

	*p_string = string;
	return true;
}
