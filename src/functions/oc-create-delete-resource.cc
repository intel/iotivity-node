#include <v8.h>
#include <node_buffer.h>

#include "../functions-internal.h"

extern "C" {
#include <ocstack.h>
#include "../callback-info.h"
}

using namespace v8;
using namespace node;

// Marshaller for OCEntityHandler callback
// defaultEntityHandler is placed in a closure each time someone calls OCCreateResource. Closures
// differ from one another only by the value of uuid, which is a key into the hash of JS callbacks.
// When the C API executes one of the closures, we construct a call to the JS callback identified
// by its UUID, and pass the return value from the callback back to the C API.
static void defaultEntityHandler(
		ffi_cif* cif,
		OCEntityHandlerResult *returnValueLocation,
		void**arguments,
		int uuid ) {
	Isolate *isolate = Isolate::GetCurrent();
	Local<Function> jsCallback = Local<Function>::Cast(
		Local<Object>::New( isolate, *_callbacks )->Get( uuid ) );

	// Call the JS callback
	Local<Value> jsCallbackArguments[ 2 ] = {
		Number::New( isolate, ( double )*( OCEntityHandlerFlag * )( arguments[ 0 ] ) ),
		Buffer::Use( isolate, ( char * )*( OCEntityHandlerRequest ** )( arguments[ 1 ] ),
			sizeof( OCEntityHandlerRequest ) )
	};
	Local<Value> returnValue = jsCallback->Call(
		isolate->GetCurrentContext()->Global(),
		2, jsCallbackArguments );

	VALIDATE_CALLBACK_RETURN_VALUE_TYPE( isolate, returnValue, IsNumber );

	*returnValueLocation = ( OCEntityHandlerResult )( returnValue->ToNumber()->Value() );
}

void bind_OCCreateResource( const FunctionCallbackInfo<Value>& args ) {
	Isolate *isolate = Isolate::GetCurrent();
	int uuid;
	OCResourceHandle handle = 0;
	callback_info *info = 0;

	VALIDATE_ARGUMENT_COUNT( isolate, args, 6 );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 0, IsObject );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 1, IsString );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 2, IsString );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 3, IsString );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 4, IsFunction );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 5, IsUint32 );

	uuid = _uuidCounter++;

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

	if ( !info ) {
		(isolate)->ThrowException( Exception::TypeError(
			String::NewFromUtf8( (isolate),
				"OCCreateResource: Unable to allocate C callback" ) ) );
		return;
	}

	// Store the JS callback in the list of callbacks under the given uuid
	Local<Object>::New( isolate, *_callbacks )->Set( uuid, args[ 4 ] );

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
	jsHandle->Set( String::NewFromUtf8( isolate, "uuid" ),
		Number::New( isolate, ( double )uuid ) );
	jsHandle->Set( String::NewFromUtf8( isolate, "callbackInfo" ),
		Buffer::New( isolate, ( const char * )&info, sizeof( callback_info * ) ) );
}

void bind_OCDeleteResource( const FunctionCallbackInfo<Value>& args ) {
	Isolate *isolate = Isolate::GetCurrent();
	OCResourceHandle handle;
	callback_info *info;
	int uuid;

	VALIDATE_ARGUMENT_COUNT( isolate, args, 1 );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 0, IsObject );

	Local<Object> jsHandle = args[ 0 ]->ToObject();

	handle = *( OCResourceHandle * )( Buffer::Data(
		jsHandle->Get( String::NewFromUtf8( isolate, "handle" ) )->ToObject() ) );

	uuid = jsHandle->Get( String::NewFromUtf8( isolate, "uuid" ) )->Uint32Value();
	info = *( callback_info ** )( Buffer::Data(
		jsHandle->Get( String::NewFromUtf8( isolate, "callbackInfo" ) )->ToObject() ) );

	Local<Object>::New( isolate, *_callbacks )->Delete( uuid );

	args.GetReturnValue().Set( Number::New( isolate, ( double )OCDeleteResource( handle ) ) );

	// OCDeleteResource will presumably remove the C callback, so we no longer need the closure.
	callback_info_free( info );
}
