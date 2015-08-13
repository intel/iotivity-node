#include <nan.h>
#include "oc-sid.h"

extern "C" {
#include <ocrandom.h>
}

using namespace v8;

Local<Array> js_SID( uint8_t *sid ) {
	int index;
	Local<Array> returnValue = NanNew<Array>( UUID_SIZE );

	for ( index = 0 ; index < UUID_SIZE ; index++ ) {
		returnValue->Set( index, NanNew<Number>( sid[ index ] ) );
	}
	return returnValue;
}
