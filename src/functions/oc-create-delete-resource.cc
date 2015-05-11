#include <v8.h>
#include <node_buffer.h>

#include "../typechecks.h"

extern "C" {
#include <ocstack.h>
#include "../callback-info.h"
}

using namespace v8;
using namespace node;

// Marshaller for OCEntityHandler callback
// defaultEntityHandler is placed in a closure each time someone calls OCCreateResource. Closures
// differ from one another only by the value of jsCallbackInPersistent, which is a persistent
// reference to the JS callback. When the C API executes one of the closures, we construct a call
// to the JS callback we find at jsCallbackInPersistent, and pass the return value from the
// callback back to the C API.
static void defaultEntityHandler(
		ffi_cif* cif,
		OCEntityHandlerResult *returnValueLocation,
		void**arguments,
		void *jsCallbackInPersistent ) {
	Isolate *isolate = Isolate::GetCurrent();

	// Construct arguments to the JS callback and then call it, recording its return value
	Local<Value> jsCallbackArguments[ 2 ] = {
		Number::New( isolate, ( double )*( OCEntityHandlerFlag * )( arguments[ 0 ] ) ),
		Buffer::Use( isolate, ( char * )*( OCEntityHandlerRequest ** )( arguments[ 1 ] ),
			sizeof( OCEntityHandlerRequest ) )
	};
	Local<Value> returnValue =
		Local<Function>::New( isolate, *( Persistent<Function> * )jsCallbackInPersistent )
			->Call( isolate->GetCurrentContext()->Global(), 2, jsCallbackArguments );

	VALIDATE_CALLBACK_RETURN_VALUE_TYPE( isolate, returnValue, IsNumber );

	*returnValueLocation = ( OCEntityHandlerResult )( returnValue->ToNumber()->Value() );
}

void bind_OCCreateResource( const FunctionCallbackInfo<Value>& args ) {
	Isolate *isolate = Isolate::GetCurrent();
	OCResourceHandle handle = 0;
	callback_info *info = 0;

	VALIDATE_ARGUMENT_COUNT( isolate, args, 6 );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 0, IsObject );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 1, IsString );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 2, IsString );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 3, IsString );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 4, IsFunction );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 5, IsUint32 );

	Persistent<Function> *jsCallback = new Persistent<Function>(
		isolate,
		Local<Function>::Cast( args[ 4 ] ) );

	// Create a new callback
	info = callback_info_new(

		// Location of JS callback
		( void * )jsCallback,

		// Function signature - return value
		&ffi_type_uint32,

		// Function signature - location of function
		( Marshaller )defaultEntityHandler,

		// Function signature - number of arguments
		2,

		// Function signature - arguments
		&ffi_type_uint32, &ffi_type_pointer );

	if ( !info ) {
		(isolate)->ThrowException( Exception::TypeError(
			String::NewFromUtf8( (isolate),
				"OCCreateResource: Unable to allocate C callback" ) ) );
		return;
	}

	args.GetReturnValue().Set(
		Number::New(
			isolate,
			( double )OCCreateResource(
				&handle,
				( const char * )*String::Utf8Value( args[ 1 ] ),
				( const char * )*String::Utf8Value( args[ 2 ] ),
				( const char * )*String::Utf8Value( args[ 3 ] ),
				( OCEntityHandler )( info->resultingFunction ),
				( uint8_t )args[ 5 ]->ToUint32()->Value() ) ) );

	Local<Object> jsHandle = args[ 0 ]->ToObject();

	jsHandle->Set( String::NewFromUtf8( isolate, "handle" ),
		Buffer::New( isolate, ( const char * )&handle, sizeof( OCResourceHandle ) ) );
	jsHandle->Set( String::NewFromUtf8( isolate, "jsCallback" ),
		Buffer::New( isolate, ( const char * )&jsCallback, sizeof( Persistent<Function> * ) ) );
	jsHandle->Set( String::NewFromUtf8( isolate, "callbackInfo" ),
		Buffer::New( isolate, ( const char * )&info, sizeof( callback_info * ) ) );
}

void bind_OCDeleteResource( const FunctionCallbackInfo<Value>& args ) {
	Isolate *isolate = Isolate::GetCurrent();

	VALIDATE_ARGUMENT_COUNT( isolate, args, 1 );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 0, IsObject );

	Local<Object> jsHandle = args[ 0 ]->ToObject();

	args.GetReturnValue().Set(
		Number::New( isolate, OCDeleteResource(
			*( OCResourceHandle * )( Buffer::Data(
				jsHandle->Get( String::NewFromUtf8( isolate, "handle" ) )
					->ToObject() ) ) ) ) );

	// OCDeleteResource will presumably remove the C callback, so we no longer need the closure.
	Local<Value> callbackInfo = jsHandle->Get( String::NewFromUtf8( isolate, "callbackInfo" ) );
	if ( !Buffer::HasInstance( callbackInfo ) ) {
		THROW_TYPECHECK_EXCEPTION( isolate, "callbackInfo is not a Node::Buffer" );
		return;
	}
	callback_info_free( *( callback_info ** )( Buffer::Data( callbackInfo->ToObject() ) ) );

	// Remove our reference to the JS callback
	Local<Value> jsCallback = jsHandle->Get( String::NewFromUtf8( isolate, "jsCallback" ) );
	if ( !Buffer::HasInstance( jsCallback ) ) {
		THROW_TYPECHECK_EXCEPTION( isolate, "jsCallback is not a Node::Buffer" );
		return;
	}
	delete *( Persistent<Function> ** )( Buffer::Data( jsCallback->ToObject() ) );
}
