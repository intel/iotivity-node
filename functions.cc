#include <node.h>
#include <node_buffer.h>
#include <v8.h>

#define VALIDATE_ARGUMENT_COUNT( isolate, args, length ) \
	if ( (args).Length() < (length) ) { \
		(isolate)->ThrowException( Exception::TypeError( \
			String::NewFromUtf8( (isolate), "Need " #length " arguments" ) ) ); \
		return; \
	}

#define VALIDATE_ARGUMENT_TYPE( isolate, args, index, typecheck  ) \
	if ( !(args)[ (index) ]->typecheck() ) { \
		(isolate)->ThrowException( Exception::TypeError( \
			String::NewFromUtf8( (isolate), "Argument " #index " must satisfy " #typecheck "()" ) ) ); \
		return; \
	}

extern "C" {

#include <ocstack.h>

}

using namespace v8;
using namespace node;

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

void InitFunctions( Handle<Object> exports ) {
	NODE_SET_METHOD( exports, "OCInit", bind_OCInit );
	NODE_SET_METHOD( exports, "OCStop", bind_OCStop );
}
