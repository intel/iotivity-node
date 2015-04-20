#include <node.h>
#include <node_buffer.h>
#include <v8.h>

#define VALIDATE_ARGUMENT_COUNT( isolate, args, length ) \
	if ( (args).Length() < (length) ) { \
		(isolate)->ThrowException( Exception::TypeError( \
			String::NewFromUtf8( (isolate), "Need " #length " arguments" ) ) ); \
		return; \
	}

#define VALIDATE_ARGUMENT_TYPE( isolate, args, index, typecheck  ) \
	if ( !(args)[ (index) ]->typecheck() ) { \
		(isolate)->ThrowException( Exception::TypeError( \
			String::NewFromUtf8( (isolate), "Argument " #index " must satisfy " #typecheck "()" ) ) ); \
		return; \
	}

extern "C" {
}

using namespace v8;
using namespace node;

void InitFunctions( Handle<Object> exports ) {
}
