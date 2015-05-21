#include <v8.h>
#include <node_buffer.h>
#include <nan.h>
#include <map>

#include "../common.h"
#include "../structures.h"

extern "C" {
#include <ocstack.h>
#include "../callback-info.h"
}

using namespace v8;
using namespace node;

// Associate the callback info with a resource handle
static std::map<OCResourceHandle, callback_info *> annotation;

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

	// Construct arguments to the JS callback and then call it, recording its return value
	Local<Value> jsCallbackArguments[ 2 ] = {
		NanNew<Number>( ( double )*( OCEntityHandlerFlag * )( arguments[ 0 ] ) ),
		NanBufferUse( ( char * )*( OCEntityHandlerRequest ** )( arguments[ 1 ] ),
			sizeof( OCEntityHandlerRequest ) )
	};
	Local<Value> returnValue =
		NanMakeCallback(
			NanGetCurrentContext()->Global(),
			NanNew( *( Persistent<Function> * )jsCallbackInPersistent ),
			2,
			jsCallbackArguments );

	VALIDATE_CALLBACK_RETURN_VALUE_TYPE( returnValue, IsNumber, "OCEntityHandler" );

	*returnValueLocation = ( OCEntityHandlerResult )( returnValue->ToNumber()->Value() );
}

NAN_METHOD( bind_OCCreateResource ) {
	NanScope();

	OCResourceHandle handle = 0;
	callback_info *info = 0;

	VALIDATE_ARGUMENT_COUNT( args, 6 );
	VALIDATE_ARGUMENT_TYPE( args, 0, IsObject );
	VALIDATE_ARGUMENT_TYPE( args, 1, IsString );
	VALIDATE_ARGUMENT_TYPE( args, 2, IsString );
	VALIDATE_ARGUMENT_TYPE( args, 3, IsString );
	VALIDATE_ARGUMENT_TYPE( args, 4, IsFunction );
	VALIDATE_ARGUMENT_TYPE( args, 5, IsUint32 );

	// Create a new callback
	info = callback_info_new(

		// Location of JS callback
		( void * )persistentJSCallback_new( Local<Function>::Cast( args[ 4 ] ) ),

		// Function that will delete the callback
		( UserDataRemover )persistentJSCallback_free,

		// Function signature - return value
		&ffi_type_uint32,

		// Function signature - location of function
		( Marshaller )defaultEntityHandler,

		// Function signature - number of arguments
		2,

		// Function signature - arguments
		&ffi_type_uint32, &ffi_type_pointer );

	if ( !info ) {
		NanThrowError( "OCCreateResource: Unable to allocate C callback" );
		NanReturnUndefined();
	}

	Local<Number> returnValue = NanNew<Number>( OCCreateResource(
		&handle,
		( const char * )*String::Utf8Value( args[ 1 ] ),
		( const char * )*String::Utf8Value( args[ 2 ] ),
		( const char * )*String::Utf8Value( args[ 3 ] ),
		( OCEntityHandler )( info->resultingFunction ),
		( uint8_t )args[ 5 ]->Uint32Value() ) );

	// Save info to the handle
	annotation[ handle ] = info;
	js_OCResourceHandle( args[ 0 ]->ToObject(), handle );

	NanReturnValue( returnValue );
}

NAN_METHOD( bind_OCDeleteResource ) {
	NanScope();

	OCStackResult returnValue;

	VALIDATE_ARGUMENT_COUNT( args, 1 );
	VALIDATE_ARGUMENT_TYPE( args, 0, IsObject );

	OCResourceHandle handle = c_OCResourceHandle( args[ 0 ]->ToObject() );

	if ( handle ) {
		callback_info *info = annotation[ handle ];
		annotation.erase( handle );
		returnValue = OCDeleteResource( handle );
		if ( info ) {
			callback_info_free( info );
		}
		NanReturnValue( NanNew<Number>( returnValue ) );
	} else {
		NanReturnUndefined();
	}
}
