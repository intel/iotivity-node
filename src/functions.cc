#include <node.h>
#include <node_buffer.h>
#include <v8.h>

using namespace v8;
using namespace node;

extern "C" {
#include <ocstack.h>
#include <ffi.h>
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

// Hash for storing JS callbacks
static Persistent<Object> callbacks;
static int uuidCounter = 0;

static void bind_OCInit( const FunctionCallbackInfo<Value>& args ) {
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

static void bind_OCStop( const FunctionCallbackInfo<Value>& args ) {
	Isolate *isolate = Isolate::GetCurrent();

	args.GetReturnValue().Set( Number::New( isolate, OCStop() ) );
}

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
		Local<Object>::New( isolate, callbacks )->Get( uuid ) );

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

static void bind_OCCreateResource( const FunctionCallbackInfo<Value>& args ) {
	Isolate *isolate = Isolate::GetCurrent();
	int uuid;
	OCResourceHandle handle = 0;
	OCStackResult result;
	callback_info *info = 0;

	VALIDATE_ARGUMENT_COUNT( isolate, args, 6 );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 0, IsObject );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 1, IsString );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 2, IsString );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 3, IsString );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 4, IsFunction );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 5, IsUint32 );

	uuid = uuidCounter++;

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
	Local<Object>::New( isolate, callbacks )->Set( uuid, args[ 4 ] );

	result = OCCreateResource(
		&handle,
		( const char * )*String::Utf8Value( args[ 1 ] ),
		( const char * )*String::Utf8Value( args[ 2 ] ),
		( const char * )*String::Utf8Value( args[ 3 ] ),
		( OCEntityHandler )( info->resultingFunction ),
		( uint8_t )args[ 5 ]->ToUint32()->Value() );

	Local<Object> jsHandle = args[ 0 ]->ToObject();

	// Store the value of the handle in a buffer
	jsHandle->Set( 0,
		Buffer::New( isolate, ( const char * )&handle, sizeof( OCResourceHandle ) ) );
	jsHandle->Set( 1, Number::New( isolate, ( double )uuid ) );

	// Store the pointer to the closure in a buffer
	jsHandle->Set( 2, Buffer::New( isolate, ( const char * )&info, sizeof( callback_info * ) ) );

	args.GetReturnValue().Set( Number::New( isolate, ( double )result ) );
}

static void bind_OCDeleteResource( const FunctionCallbackInfo<Value>& args ) {
	Isolate *isolate = Isolate::GetCurrent();
	OCResourceHandle handle;
	callback_info *info;
	int uuid;

	VALIDATE_ARGUMENT_COUNT( isolate, args, 1 );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 0, IsObject );

	Local<Object> jsHandle = args[ 0 ]->ToObject();

	handle = *( OCResourceHandle * )( Buffer::Data( jsHandle->Get( 0 )->ToObject() ) );
	uuid = jsHandle->Get( 1 )->Uint32Value();
	info = *( callback_info ** )( Buffer::Data( jsHandle->Get( 2 )->ToObject() ) );

	Local<Object>::New( isolate, callbacks )->Delete( uuid );

	args.GetReturnValue().Set( Number::New( isolate, ( double )OCDeleteResource( handle ) ) );

	// OCDeleteResource will presumably remove the C callback, so we no longer need the closure.
	callback_info_free( info );
}

void InitFunctions( Handle<Object> exports, Handle<Object> module ) {
	Isolate *isolate = Isolate::GetCurrent();

	// Initialize hash for storing JS callbacks
	callbacks.Reset( isolate, Object::New( isolate ) );

	NODE_SET_METHOD( exports, "OCInit", bind_OCInit );
	NODE_SET_METHOD( exports, "OCStop", bind_OCStop );
	NODE_SET_METHOD( exports, "OCCreateResource", bind_OCCreateResource );
	NODE_SET_METHOD( exports, "OCDeleteResource", bind_OCDeleteResource );

#ifdef TESTING

	// For testing purposes we assign the callbacks object to the module exports so we can inspect
	// it from the JS test suite
	exports->Set( String::NewFromUtf8( isolate, "_test_callbacks" ),
		Local<Object>::New( isolate, callbacks ) );
#endif /* TESTING */
}
