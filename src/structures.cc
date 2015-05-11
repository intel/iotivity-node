#include <node_buffer.h>

extern "C" {
#include <string.h>
}

#include "structures.h"
#include "typechecks.h"

using namespace v8;
using namespace node;

Local<Object> js_OCClientResponse( Isolate *isolate, OCClientResponse *response ) {
	uint32_t optionIndex, optionDataIndex;
	Local<Object> jsResponse = Object::New( isolate );

		// jsResponse.addr
		Local<Object> addr = Object::New( isolate );

			// jsResponse.addr.size
			addr->Set( String::NewFromUtf8( isolate, "size" ),
				Number::New( isolate, response->addr->size ) );

			// jsResponse.addr.addr
			Local<Array> addrAddr = Array::New( isolate, response->addr->size );
			for ( optionIndex = 0 ; optionIndex < response->addr->size ; optionIndex++ ) {
				addrAddr->Set( optionIndex, Number::New(
					isolate,
					response->addr->addr[ optionIndex ] ) );
			}
			addr->Set( String::NewFromUtf8( isolate, "addr" ), addrAddr );

		jsResponse->Set( String::NewFromUtf8( isolate, "addr" ), addr );

		// jsResponse.connType
		jsResponse->Set( String::NewFromUtf8( isolate, "connType" ),
			Number::New( isolate, response->connType ) );

		// jsResponse.result
		jsResponse->Set( String::NewFromUtf8( isolate, "result" ),
			Number::New( isolate, response->result ) );

		// jsResponse.sequenceNumber
		jsResponse->Set( String::NewFromUtf8( isolate, "sequenceNumber" ),
			Number::New( isolate, response->sequenceNumber ) );

		// jsResponse.resJSONPayload
		if ( response->resJSONPayload ) {
			jsResponse->Set( String::NewFromUtf8( isolate, "resJSONPayload" ),
				String::NewFromUtf8( isolate, response->resJSONPayload ) );
		}

		// jsResponse.numRcvdVendorSpecificHeaderOptions
		jsResponse->Set(
			String::NewFromUtf8( isolate, "numRcvdVendorSpecificHeaderOptions" ),
			Number::New( isolate, response->numRcvdVendorSpecificHeaderOptions ) );

		// jsResponse.rcvdVendorSpecificHeaderOptions
		Local<Array> headerOptions = Array::New(
			isolate,
			response->numRcvdVendorSpecificHeaderOptions );

			// jsResponse.rcvdVendorSpecificHeaderOptions[ index ]
			OCHeaderOption *cHeaderOptions = response->rcvdVendorSpecificHeaderOptions;
			uint8_t headerOptionCount = response->numRcvdVendorSpecificHeaderOptions;
			for ( optionIndex = 0 ;
					optionIndex < headerOptionCount ;
					optionIndex++ ) {
				Local<Object> headerOption = Object::New( isolate );

					// response.rcvdVendorSpecificHeaderOptions[ index ].protocolID
					headerOption->Set( String::NewFromUtf8( isolate, "protocolID" ),
						Number::New( isolate, cHeaderOptions[ optionIndex ].protocolID ) );

					// response.rcvdVendorSpecificHeaderOptions[ index ].optionID
					headerOption->Set( String::NewFromUtf8( isolate, "optionID" ),
						Number::New( isolate, cHeaderOptions[ optionIndex ].optionID ) );

					// response.rcvdVendorSpecificHeaderOptions[ index ].optionLength
					headerOption->Set( String::NewFromUtf8( isolate, "optionLength" ),
						Number::New( isolate, cHeaderOptions[ optionIndex ].optionLength ) );

					// response.rcvdVendorSpecificHeaderOptions[ index ].optionData
					Local<Array> headerOptionData = Array::New(
						isolate,
						cHeaderOptions[ optionIndex ].optionLength );
					for ( optionDataIndex = 0 ;
							optionDataIndex < cHeaderOptions[ optionIndex ].optionLength ;
							optionDataIndex++ ) {
						headerOptionData->Set(
							optionDataIndex,
							Number::New(
								isolate,
								cHeaderOptions[ optionIndex ].optionData[ optionDataIndex ] ) );
					}
					headerOption->Set(
						String::NewFromUtf8( isolate, "optionData" ),
						headerOptionData );
				headerOptions->Set( optionIndex, headerOption );
			}
		jsResponse->Set(
			String::NewFromUtf8( isolate, "rcvdVendorSpecificHeaderOptions" ),
			headerOptions );

	return jsResponse;
}

bool c_OCEntityHandlerResponse(
		Isolate *isolate,
		OCEntityHandlerResponse *destination,
		v8::Local<Object> jsOCEntityHandlerResponse ) {

	// requestHandle
	Local<Value> requestHandle =
		jsOCEntityHandlerResponse->Get( String::NewFromUtf8( isolate, "requestHandle" ) );
	if ( !Buffer::HasInstance( requestHandle ) ) {
		THROW_TYPECHECK_EXCEPTION( isolate, "requestHandle is not a Node::Buffer" );
		return false;
	}
	destination->requestHandle = *( OCRequestHandle * )Buffer::Data( requestHandle->ToObject() );

	// responseHandle is filled in by the stack

	// resourceHandle
	Local<Value> resourceHandle =
		jsOCEntityHandlerResponse->Get( String::NewFromUtf8( isolate, "resourceHandle" ) );
	if ( !Buffer::HasInstance( resourceHandle ) ) {
		THROW_TYPECHECK_EXCEPTION( isolate, "resourceHandle is not a Node::Buffer" );
		return false;
	}
	destination->resourceHandle = *( OCResourceHandle * )Buffer::Data( resourceHandle->ToObject() );

	// ehResult
	Local<Value> ehResult = jsOCEntityHandlerResponse->Get( String::NewFromUtf8( isolate, "ehResult" ) );
	VALIDATE_VALUE_TYPE( isolate, ehResult, IsUint32, "ehResult", false );
	destination->ehResult = ( OCEntityHandlerResult )ehResult->Uint32Value();

	// payload
	Local<Value> payload = jsOCEntityHandlerResponse->Get( String::NewFromUtf8( isolate, "payload" ) );
	VALIDATE_VALUE_TYPE( isolate, payload, IsString, "payload", false );
	const char *payloadString = ( const char * )*String::Utf8Value( payload );
	size_t payloadLength = strlen( payloadString );
	if ( payloadLength >= MAX_RESPONSE_LENGTH ) {
		THROW_RANGE_EXCEPTION( isolate, "payload is longer than MAX_RESPONSE_LENGTH" );
		return false;
	}
	strncpy( destination->payload, payloadString, payloadLength );

	// payloadSize
	destination->payloadSize = payloadLength;

	// numSendVendorSpecificHeaderOptions
	Local<Value> numSendVendorSpecificHeaderOptions = jsOCEntityHandlerResponse->Get(
		String::NewFromUtf8( isolate, "numSendVendorSpecificHeaderOptions" ) );
	VALIDATE_VALUE_TYPE(
		isolate,
		numSendVendorSpecificHeaderOptions,
		IsUint32,
		"numSendVendorSpecificHeaderOptions",
		false );
	uint8_t headerOptionCount = ( uint8_t )numSendVendorSpecificHeaderOptions->Uint32Value();
	if ( headerOptionCount > MAX_HEADER_OPTIONS ) {
		THROW_RANGE_EXCEPTION( isolate,
			"numSendVendorSpecificHeaderOptions is larger than MAX_HEADER_OPTIONS" );
		return false;
	}
	destination->numSendVendorSpecificHeaderOptions = headerOptionCount;

	// sendVendorSpecificHeaderOptions
	int headerOptionIndex, optionDataIndex;
	Local<Value> headerOptionsValue = jsOCEntityHandlerResponse->Get(
		String::NewFromUtf8( isolate, "sendVendorSpecificHeaderOptions" ) );
	VALIDATE_VALUE_TYPE(
		isolate,
		headerOptionsValue,
		IsArray,
		"sendVendorSpecificHeaderOptions",
		false );
	Local<Array> headerOptions = Local<Array>::Cast( headerOptionsValue );
	for ( headerOptionIndex = 0 ; headerOptionIndex < headerOptionCount; headerOptionIndex++ ) {
		Local<Value> headerOptionValue = headerOptions->Get( headerOptionIndex );
		VALIDATE_VALUE_TYPE(
			isolate,
			headerOptionValue,
			IsObject,
			"sendVendorSpecificHeaderOptions member",
			false );
		Local<Object> headerOption = headerOptionValue->ToObject();

		// sendVendorSpecificHeaderOptions[].protocolID
		Local<Value> protocolIDValue = headerOption->Get(
			String::NewFromUtf8( isolate, "protocolID" ) );
		VALIDATE_VALUE_TYPE( isolate, protocolIDValue, IsUint32, "protocolID", false );
		destination->sendVendorSpecificHeaderOptions[ headerOptionIndex ].protocolID =
			( OCTransportProtocolID )protocolIDValue->Uint32Value();

		// sendVendorSpecificHeaderOptions[].optionID
		Local<Value> optionIDValue = headerOption->Get(
			String::NewFromUtf8( isolate, "optionID" ) );
		VALIDATE_VALUE_TYPE( isolate, optionIDValue, IsUint32, "optionID", false );
		destination->sendVendorSpecificHeaderOptions[ headerOptionIndex ].optionID =
			( uint16_t )protocolIDValue->Uint32Value();

		// sendVendorSpecificHeaderOptions[].optionLength
		Local<Value> optionLengthValue = headerOption->Get(
			String::NewFromUtf8( isolate, "optionLength" ) );
		VALIDATE_VALUE_TYPE( isolate, optionLengthValue, IsUint32, "optionLength", false );
		uint16_t optionLength = ( uint16_t )optionLengthValue->Uint32Value();
		if ( optionLength > MAX_HEADER_OPTION_DATA_LENGTH ) {
			THROW_RANGE_EXCEPTION( isolate,
				"optionLength is larger than MAX_HEADER_OPTION_DATA_LENGTH" );
			return false;
		}
		destination->sendVendorSpecificHeaderOptions[ headerOptionIndex ].optionLength =
			optionLength;

		// sendVendorSpecificHeaderOptions[].optionData
		Local<Value> optionDataValue = headerOption->Get(
			String::NewFromUtf8( isolate, "optionData" ) );
		VALIDATE_VALUE_TYPE( isolate, optionDataValue, IsArray, "optionData", false );
		Local<Array> optionData = Local<Array>::Cast( optionDataValue );
		for ( optionDataIndex = 0 ; optionDataIndex < optionLength ; optionDataIndex++ ) {
			Local<Value> optionDataItemValue = optionData->Get( optionDataIndex );
			VALIDATE_VALUE_TYPE(
				isolate,
				optionDataItemValue,
				IsUint32,
				"optionData item",
				false );
			destination->sendVendorSpecificHeaderOptions[ headerOptionIndex ]
				.optionData[ optionDataIndex ] = ( uint8_t )optionDataItemValue->Uint32Value();
		}
	}

	return true;
}
