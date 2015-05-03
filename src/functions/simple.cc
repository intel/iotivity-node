#include <v8.h>
#include "../functions-internal.h"

extern "C" {
#include <ocstack.h>
}

using namespace v8;

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
	args.GetReturnValue().Set( Number::New( Isolate::GetCurrent(), OCStop() ) );
}

void bind_OCProcess( const FunctionCallbackInfo<Value>& args ) {
	args.GetReturnValue().Set( Number::New( Isolate::GetCurrent(), OCProcess() ) );
}

void bind_OCStartPresence( const FunctionCallbackInfo<Value>& args ) {
	Isolate *isolate = Isolate::GetCurrent();

	VALIDATE_ARGUMENT_COUNT( isolate, args, 1 );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 0, IsUint32 );

	args.GetReturnValue().Set( Number::New( isolate,
		OCStartPresence( ( uint32_t )args[ 0 ]->Uint32Value() ) ) );
}

void bind_OCStopPresence( const FunctionCallbackInfo<Value>& args ) {
	args.GetReturnValue().Set( Number::New( Isolate::GetCurrent(), OCStopPresence() ) );
}
