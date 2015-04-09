#include <node.h>
#include <v8.h>

extern "C" {

#include <occoaphelper.h>

}

using namespace v8;

void bind_hello( const FunctionCallbackInfo<Value>& args ) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope( isolate );

	args.GetReturnValue().Set( String::NewFromUtf8( isolate, "world" ) );
}

void bind_CoAPToOCResponseCode( const FunctionCallbackInfo<Value>& args ) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope( isolate );

	if ( args.Length() < 1 ) {
		isolate->ThrowException( Exception::TypeError(
			String::NewFromUtf8( isolate, "Need coapCode" ) ) );
		return;
	}

	if ( !args[ 0 ]->IsNumber() ) {
		isolate->ThrowException( Exception::TypeError(
			String::NewFromUtf8( isolate, "coapCode must be a number" ) ) );
		return;
	}

	args.GetReturnValue().Set(
		Number::New( isolate,
			( double )CoAPToOCResponseCode( ( uint8_t )args[ 0 ]->NumberValue() ) ) );
}

void Init( Handle<Object> exports ) {
	Isolate* isolate = Isolate::GetCurrent();

	exports->Set( String::NewFromUtf8( isolate, "hello" ),
		FunctionTemplate::New( isolate, bind_hello )->GetFunction() );

	exports->Set( String::NewFromUtf8( isolate, "CoAPToOCResponseCode" ),
		FunctionTemplate::New( isolate, bind_CoAPToOCResponseCode )->GetFunction() );
}

NODE_MODULE( iotivity, Init )
