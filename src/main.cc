#include <node.h>
#include <v8.h>

extern "C" {
#include <ocstack.h>
}

#include "functions.h"
#include "enums.h"

using namespace v8;

void Init( Handle<Object> exports, Handle<Object> module ) {
	Isolate *isolate = Isolate::GetCurrent();

	InitFunctions( exports, module );
	InitEnums( exports );

	exports->Set(
		String::NewFromUtf8( isolate, "MAX_RESPONSE_LENGTH" ),
		Number::New( isolate, MAX_RESPONSE_LENGTH ) );

	exports->Set(
		String::NewFromUtf8( isolate, "MAX_URI_LENGTH" ),
		Number::New( isolate, MAX_URI_LENGTH ) );

	exports->Set(
		String::NewFromUtf8( isolate, "MAX_HEADER_OPTIONS" ),
		Number::New( isolate, MAX_HEADER_OPTIONS ) );

	exports->Set(
		String::NewFromUtf8( isolate, "MAX_HEADER_OPTION_DATA_LENGTH" ),
		Number::New( isolate, MAX_HEADER_OPTION_DATA_LENGTH ) );
}

NODE_MODULE( iotivity, Init )
