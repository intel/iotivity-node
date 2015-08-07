#include <nan.h>
#include "oc-client-response.h"
#include "oc-dev-addr.h"
#include "../common.h"
#include "oc-header-option-array.h"
#include "oc-payload.h"

extern "C" {
#include <string.h>
}

using namespace v8;

Local<Object> js_OCClientResponse( OCClientResponse *response ) {
	Local<Object> returnValue = NanNew<Object>();

	// response.devAddr
	returnValue->Set( NanNew<String>( "devAddr" ), js_OCDevAddr( &( response->devAddr ) ) );

	// response.addr
	returnValue->Set( NanNew<String>( "addr" ), js_OCDevAddr( response->addr ) );

	// response.connType
	returnValue->Set( NanNew<String>( "connType" ), NanNew<Number>( response->connType ) );

	// response.result
	returnValue->Set( NanNew<String>( "result" ), NanNew<Number>( response->result ) );

	// response.sequenceNumber
	returnValue->Set( NanNew<String>( "sequenceNumber" ), NanNew<Number>( response->sequenceNumber ) );

	// response.resourceUri
	returnValue->Set( NanNew<String>( "resourceUri" ), NanNew<String>( response->resourceUri ) );

	// response.payload
	returnValue->Set( NanNew<String>( "payload" ), js_OCPayload( response->payload ) );

	// response.rcvdVendorSpecificHeaderOptions
	returnValue->Set( NanNew<String>( "rcvdVendorSpecificHeaderOptions" ),
		js_OCHeaderOption( response->rcvdVendorSpecificHeaderOptions, response->numRcvdVendorSpecificHeaderOptions ) );

	return returnValue;
}
