#include <node.h>
#include <node_buffer.h>
#include <v8.h>

using namespace v8;
using namespace node;

extern "C" {
#include <ocstack.h>
#include <ffi.h>

#ifdef TESTING
#include <stdint.h>
#endif // TESTING

#include "callback-info.h"
}

#define THROW_TYPECHECK_EXCEPTION( isolate, typecheck, message ) \
		(isolate)->ThrowException( Exception::TypeError( \
			String::NewFromUtf8( (isolate), \
				message " must satisfy " #typecheck "()" ) ) )

#define VALIDATE_CALLBACK_RETURN_VALUE_TYPE( isolate, value, typecheck ) \
	if ( !value->typecheck() ) { \
		THROW_TYPECHECK_EXCEPTION( isolate, typecheck, "Callback return value type" ); \
	}

#define VALIDATE_ARGUMENT_COUNT( isolate, args, length ) \
	if ( (args).Length() < (length) ) { \
		(isolate)->ThrowException( Exception::TypeError( \
			String::NewFromUtf8( (isolate), "Need " #length " arguments" ) ) ); \
		return; \
	}

#define VALIDATE_ARGUMENT_TYPE( isolate, args, index, typecheck  ) \
	if ( !(args)[ (index) ]->typecheck() ) { \
		THROW_TYPECHECK_EXCEPTION( isolate, typecheck, "Argument " #index " " ); \
		return; \
	}

Persistent<Object> module_exports;

#define JS_CALLBACK( isolate, uuid ) \
	Local<Object>::New( (isolate), module_exports ) \
		->Get( String::NewFromUtf8( (isolate), "_callbacks" ) )->ToObject() \
			->Get( String::NewFromUtf8( (isolate), "callbacks" ) )->ToObject() \
				->Get( (uuid) )

#ifdef TESTING
#define JS_LOG( isolate ) \
	Local<Function>::Cast( \
		Local<Object>::New( (isolate), module_exports ) \
			->Get( String::NewFromUtf8( (isolate), "_log" ) ) )
#endif // TESTING

void bind_OCInit( const FunctionCallbackInfo<Value>& args ) {
	Isolate *isolate = Isolate::GetCurrent();

	VALIDATE_ARGUMENT_COUNT( isolate, args, 3 );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 0, IsString );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 1, IsUint32 );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 2, IsNumber );

	args.GetReturnValue().Set( Number::New( isolate,
		OCInit(
			( const char * )*String::Utf8Value( args[ 0 ] ),
			( uint16_t )args[ 1 ]->ToUint32()->Value(),
			( OCMode )args[ 2 ]->ToNumber()->Value() ) ) );
}

void bind_OCStop( const FunctionCallbackInfo<Value>& args ) {
	Isolate *isolate = Isolate::GetCurrent();

	args.GetReturnValue().Set( Number::New( isolate, OCStop() ) );
}

void dispose_of_callback_info( const v8::WeakCallbackData<v8::Value, _callback_info>& data ) {
	callback_info_free( ( callback_info * )data.GetParameter() );

#ifdef TESTING
	Isolate *isolate = Isolate::GetCurrent();
	Local<Value> logArguments[ 2 ] = {
		String::NewFromUtf8( isolate, "callback_info_free" ),
		Number::New( isolate, ( double )( uintptr_t )data.GetParameter() )
	};
	JS_LOG( isolate )->Call( isolate->GetCurrentContext()->Global(), 2, logArguments );
#endif
}

// Marshaller for OCEntityHandler callback
// defaultEntityHandler is placed in a closure each time someone calls OCCreateResource. Closures
// differ from one another only by the value of uuid, which is a key into the hash of JS callbacks.
// When the C API executes one of the closures, we construct a call to the JS callback identified
// by its UUID, and pass the return value from the callback back to the C API.
void defaultEntityHandler(
		ffi_cif* cif,
		OCEntityHandlerResult *returnValueLocation,
		void**arguments,
		int uuid ) {
	Isolate *isolate = Isolate::GetCurrent();
	Local<Value> jsCallback = JS_CALLBACK( isolate, uuid );

	// Make sure the JS callback is present
	if ( !jsCallback->IsFunction() ) {
		isolate->ThrowException( Exception::TypeError(
			String::Concat(
				String::Concat(
					String::NewFromUtf8( isolate, "JS Callback with UUID " ),
					String::NewFromUtf8( isolate,
						*String::Utf8Value( Number::New( isolate, ( double )uuid ) ) ) ),
				String::NewFromUtf8( isolate, " not found" ) ) ) );
		return;
	}

	// Call the JS callback
	Local<Value> jsCallbackArguments[ 2 ] = {
		Number::New( isolate, ( double )*( OCEntityHandlerFlag * )( arguments[ 0 ] ) ),
		Buffer::Use( isolate, ( char * )*( OCEntityHandlerRequest ** )( arguments[ 1 ] ),
			sizeof( OCEntityHandlerRequest ) )
	};
	Local<Value> returnValue = Local<Function>::Cast( jsCallback )->Call(
		isolate->GetCurrentContext()->Global(),
		2, jsCallbackArguments );

	VALIDATE_CALLBACK_RETURN_VALUE_TYPE( isolate, returnValue, IsNumber );

	*returnValueLocation = ( OCEntityHandlerResult )( returnValue->ToNumber()->Value() );
}

void bind__partial_OCCreateResource( const FunctionCallbackInfo<Value>& args ) {
	Isolate *isolate = Isolate::GetCurrent();
	int uuid;
	OCResourceHandle handle = 0;
	OCStackResult result;
	Handle<Array> returnArray;
	callback_info *info;

	VALIDATE_ARGUMENT_COUNT( isolate, args, 5 );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 0, IsString );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 1, IsString );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 2, IsString );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 3, IsUint32 ); // uuid used as key into callbacks array
	VALIDATE_ARGUMENT_TYPE( isolate, args, 4, IsUint32 );

	uuid = args[ 3 ]->ToUint32()->Value();

	// Create a new callback
	info = callback_info_new(

		// Location of JS callback
		uuid,

		// Function signature - return value
		&ffi_type_uint32,

		// Function signature - location of function
		( Marshaller )defaultEntityHandler,

		// Function signature - number of arguments
		2,

		// Function signature - arguments
		&ffi_type_uint32, &ffi_type_pointer );

#ifdef TESTING
	Local<Value> logArguments[ 2 ] = {
		String::NewFromUtf8( isolate, "callback_info_new" ),
		Number::New( isolate, ( double )( uintptr_t )info )
	};
	JS_LOG( isolate )->Call( isolate->GetCurrentContext()->Global(), 2, logArguments );
#endif

	if ( !info ) {
		(isolate)->ThrowException( Exception::TypeError(
			String::NewFromUtf8( (isolate),
				"OCCreateResource: Unable to allocate C callback" ) ) );
		return;
	}

	result = OCCreateResource(
		&handle,
		( const char * )*String::Utf8Value( args[ 0 ] ),
		( const char * )*String::Utf8Value( args[ 1 ] ),
		( const char * )*String::Utf8Value( args[ 2 ] ),
		( OCEntityHandler )( info->resultingFunction ),
		( uint8_t )args[ 4 ]->ToUint32()->Value() );

	// We return an array containing the result code and the handle. It is made weakly
	// persistent so as to allow us to dispose of the callback_info structure when it's no
	// longer in use.
	returnArray = Array::New( isolate, 3 );
	returnArray->Set( 0, Number::New( isolate, ( double )result ) );
	returnArray->Set( 1,
		Buffer::New( isolate, ( const char * )&handle, sizeof( OCResourceHandle ) ) );
	returnArray->Set( 2, Number::New( isolate, ( double )uuid ) );

	Persistent<Value> jsReturnValue( isolate, returnArray );

	jsReturnValue.SetWeak( info, dispose_of_callback_info );

	args.GetReturnValue().Set( jsReturnValue );
}

void bind__partial_OCDeleteResource( const FunctionCallbackInfo<Value>& args ) {
	Isolate *isolate = Isolate::GetCurrent();

	VALIDATE_ARGUMENT_COUNT( isolate, args, 1 );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 0, IsArray );

	args.GetReturnValue().Set(
		Number::New( isolate, ( double )OCDeleteResource(
			( OCResourceHandle )Buffer::Data(
				Handle<Array>::Cast( args[ 0 ] )->Get( 1 )->ToObject() ) ) ) );
}

void InitFunctions( Handle<Object> exports, Handle<Object> module ) {
	module_exports.Reset( Isolate::GetCurrent(), exports );

	NODE_SET_METHOD( exports, "OCInit", bind_OCInit );
	NODE_SET_METHOD( exports, "OCStop", bind_OCStop );
	NODE_SET_METHOD( exports, "_partial_OCCreateResource", bind__partial_OCCreateResource );
	NODE_SET_METHOD( exports, "_partial_OCDeleteResource", bind__partial_OCDeleteResource );
}
