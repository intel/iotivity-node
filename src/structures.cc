#include "structures.h"

using namespace v8;

Local<Object> jsOCClientResponse( Isolate *isolate, OCClientResponse *response ) {
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
