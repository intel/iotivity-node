#include <v8.h>
#include <node_buffer.h>
#include <nan.h>

#include "../common.h"
#include "../structures.h"

extern "C" {
#include <ocstack.h>
#include "../callback-info.h"
}

using namespace v8;
using namespace node;

NAN_METHOD( bind_OCNotifyAllObservers ) {
	NanScope();

	VALIDATE_ARGUMENT_COUNT( args, 2 );
	VALIDATE_ARGUMENT_TYPE( args, 0, IsObject );
	VALIDATE_ARGUMENT_TYPE( args, 1, IsNumber );

	OCResourceHandle handle = c_OCResourceHandle( args[ 0 ]->ToObject() );

	if ( handle ) {
		NanReturnValue( NanNew<Number>( OCNotifyAllObservers(
			handle,
			( OCQualityOfService )args[ 1 ]->Uint32Value() ) ) );
	} else {
		NanReturnUndefined();
	}
}
