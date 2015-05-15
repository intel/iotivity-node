#include <node_buffer.h>
#include "oc-do-resource.h"
#include "../common.h"
#include "../structures.h"

extern "C" {
#include <stdlib.h>
#include <ocstack.h>
}

using namespace v8;
using namespace node;

void bind_OCDoResponse( const FunctionCallbackInfo<Value>& args ) {
	Isolate *isolate = Isolate::GetCurrent();
	OCEntityHandlerResponse response;
	char payload[ MAX_RESPONSE_LENGTH ];

	response.payload = payload;

	VALIDATE_ARGUMENT_COUNT( isolate, args, 1 );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 0, IsObject );

	if ( c_OCEntityHandlerResponse( isolate, &response, args[ 0 ]->ToObject() ) ) {
		args.GetReturnValue().Set( Number::New( isolate, OCDoResponse( &response ) ) );
	}
}
