#include <nan.h>
#include "../common.h"
#include "oc-payload.h"

extern "C" {
#include <string.h>
}

// FIXME: Remove the definition below when the stack starts providing it
#define UUID_SIZE (16)

using namespace v8;

Local<Object> js_OCResourcePayload( OCResourcePayload *payload ) {
	int index, length;
	OCStringLL *item;
	Local<Object> returnValue = NanNew<Object>();

	// payload.uri
	returnValue->Set( NanNew<String>( "uri" ), NanNew<String>( payload->uri ) );

	// payload.sid
	Local<Array> sid = NanNew<Array>( UUID_SIZE );
	for ( index = 0 ; index < UUID_SIZE ; index++ ) {
		sid->Set( index, NanNew<Number>( payload->sid[ index ] ) );
	}
	returnValue->Set( NanNew<String>( "sid" ), sid );

	// payload.types
	for ( item = payload->types, length = 0; item; item = item->next, length++ );
	Local<Array> types = NanNew<Array>( length );
	for ( item = payload->types, index = 0; item; item = item->next, index++ ) {
		types->Set( index, NanNew<String>( item->value ) );
	}
	returnValue->Set( NanNew<String>( "types" ), types );

	// payload.interfaces
	for ( item = payload->interfaces, length = 0; item; item = item->next, length++ );
	Local<Array> interfaces = NanNew<Array>( length );
	for ( item = payload->interfaces, index = 0; item; item = item->next, index++ ) {
		interfaces->Set( index, NanNew<String>( item->value ) );
	}
	returnValue->Set( NanNew<String>( "interfaces" ), interfaces );

	// payload.bitmap
	returnValue->Set( NanNew<String>( "bitmap" ), NanNew<Number>( payload->bitmap ) );

	// payload.secure
	returnValue->Set( NanNew<String>( "secure" ), NanNew<Boolean>( payload->secure ) );

	// payload.port
	returnValue->Set( NanNew<String>( "port" ), NanNew<Number>( payload->port ) );

	return returnValue;
}

Local<Object> js_OCDiscoveryPayload( OCDiscoveryPayload *payload ) {
	Local<Object> returnValue = NanNew<Object>();
	OCResourcePayload *resource = payload->resources;
	int counter = 0;

	returnValue->Set( NanNew<String>( "type" ), NanNew<Number>( payload->base.type ) );

	// Count the resources
	while( resource ) {
		counter++;
		resource = resource->next;
	}

	Local<Array> resources = NanNew<Array>( counter );

	for ( resource = payload->resources, counter = 0; resource; resource = resource->next, counter++ ) {
		resources->Set( counter, js_OCResourcePayload( resource ) );
	}

	returnValue->Set( NanNew<String>( "resources" ), resources );

	return returnValue;
}

Local<Value> js_OCPayload( OCPayload *payload ) {
	switch( payload->type ) {
		case PAYLOAD_TYPE_DISCOVERY:
			return js_OCDiscoveryPayload( ( OCDiscoveryPayload * )payload );
			break;

		case PAYLOAD_TYPE_INVALID:
		default:
			break;
	}
	return NanNull();
}
