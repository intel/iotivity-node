#include <nan.h>
#include "handles.h"
#include "../common.h"

using namespace v8;

static Local<Array> jsArrayFromBytes( char *bytes, size_t length ) {
	size_t index;
	Local<Array> returnValue = NanNew<Array>( length );

	for ( index = 0 ; index < length ; index++ ) {
		returnValue->ForceSet(
			NanNew<Uint32>( index ),
			NanNew<Uint32>( bytes[ index ] ),
			( PropertyAttribute )( ReadOnly | DontEnum | DontDelete ) );
	}
	return returnValue;
}

static bool fillCArrayFromJSArray( char *bytes, size_t length, Local<Array> array ) {
	size_t index, arrayLength;

	arrayLength = array->Length();
	if ( arrayLength != length ) {
		NanThrowError( "byte array has the wrong length" );
		return false;
	}

	for ( index = 0 ; index < length ; index++ ) {
		Local<Value> byte = array->Get( index );
		VALIDATE_VALUE_TYPE( byte, IsUint32, "byte array item", false );
		PropertyAttribute attributes = array->GetPropertyAttributes( NanNew<Uint32>( index ) );
		if ( !( attributes & ( ReadOnly | DontEnum | DontDelete ) ) ) {
			NanThrowError( "byte array item has been tampered with" );
			return false;
		}
		bytes[ index ] = ( char )( byte->Uint32Value() );
	}

	return true;
}

Local<Array> js_OCResourceHandle( OCResourceHandle handle ) {
	return jsArrayFromBytes( ( ( char * )( &handle ) ), sizeof( OCResourceHandle ) );
}

bool c_OCResourceHandle( Local<Array> handle, OCResourceHandle *p_cHandle ) {
	OCResourceHandle local;

	if ( !fillCArrayFromJSArray( ( ( char * )&local ), sizeof( OCResourceHandle ), handle ) ) {
		return false;
	}

	*p_cHandle = local;
	return true;
}

Local<Array> js_OCRequestHandle( OCRequestHandle handle ) {
	return jsArrayFromBytes( ( ( char * )( &handle ) ), sizeof( OCRequestHandle ) );
}

bool c_OCRequestHandle( Local<Array> handle, OCRequestHandle *p_cHandle ) {
	OCRequestHandle local;

	if ( !fillCArrayFromJSArray( ( ( char * )&local ), sizeof( OCRequestHandle ), handle ) ) {
		return false;
	}

	*p_cHandle = local;
	return true;
}

Local<Array> js_OCDoHandle( OCDoHandle handle ) {
	return jsArrayFromBytes( ( ( char * )( &handle ) ), sizeof( OCDoHandle ) );
}

bool c_OCDoHandle( Local<Array> handle, OCDoHandle *p_cHandle ) {
	OCDoHandle local;

	if ( !fillCArrayFromJSArray( ( ( char * )&local ), sizeof( OCDoHandle ), handle ) ) {
		return false;
	}

	*p_cHandle = local;
	return true;
}
