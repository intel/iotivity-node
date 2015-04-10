#include <node.h>
#include <v8.h>

extern "C" {

#include <occoaphelper.h>

}

using namespace v8;

void bind_hello( const FunctionCallbackInfo<Value>& args ) {
	Isolate* isolate = Isolate::GetCurrent();

	args.GetReturnValue().Set( String::NewFromUtf8( isolate, "world" ) );
}

void bind_CoAPToOCResponseCode( const FunctionCallbackInfo<Value>& args ) {
	Isolate* isolate = Isolate::GetCurrent();

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
			( double )CoAPToOCResponseCode( ( uint8_t )args[ 0 ]->Uint32Value() ) ) );
}

void bind_CreateNewOptionNode( const FunctionCallbackInfo<Value>& args ) {
	Isolate* isolate = Isolate::GetCurrent();

	if ( args.Length() < 3 ) {
		isolate->ThrowException( Exception::TypeError(
			String::NewFromUtf8( isolate, "Need key, length, and data" ) ) );
		return;
	}

	if ( !args[ 0 ]->IsNumber() ) {
		isolate->ThrowException( Exception::TypeError(
			String::NewFromUtf8( isolate, "key must be a number" ) ) );
		return;
	}

	if ( !args[ 1 ]->IsNumber() ) {
		isolate->ThrowException( Exception::TypeError(
			String::NewFromUtf8( isolate, "length must be a number" ) ) );
		return;
	}

	if ( !args[ 2 ]->IsArray() ) {
		isolate->ThrowException( Exception::TypeError(
			String::NewFromUtf8( isolate, "key must be an array" ) ) );
		return;
	}

	args.GetReturnValue().Set(
		Buffer::New( isolate,
			CreateNewOptionNode(
				( unsigned short )args[ 0 ]->ToUint32(),
				( unsigned int )args[ 1 ]->ToUint32(),
				Buffer::Data( args[ 2 ]->ToObject() ) ),
			length ) );
}

void InitFunctions( Handle<Object> exports ) {
	NODE_SET_METHOD( exports, "hello", bind_hello );
	NODE_SET_METHOD( exports, "CoAPToOCResponseCode", bind_CoAPToOCResponseCode );
	NODE_SET_METHOD( exports, "CreateNewOptionNode", bind_CreateNewOptionNode );
}
