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

#include <occoaphelper.h>

}

using namespace v8;
using namespace node;

void bind_hello( const FunctionCallbackInfo<Value>& args ) {
	Isolate* isolate = Isolate::GetCurrent();

	args.GetReturnValue().Set( String::NewFromUtf8( isolate, "world" ) );
}

void bind_CoAPToOCResponseCode( const FunctionCallbackInfo<Value>& args ) {
	Isolate* isolate = Isolate::GetCurrent();

	VALIDATE_ARGUMENT_COUNT( isolate, args, 1 );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 0, IsNumber );

	args.GetReturnValue().Set(
		Number::New( isolate,
			( double )CoAPToOCResponseCode( ( uint8_t )args[ 0 ]->Uint32Value() ) ) );
}

void bind_CreateNewOptionNode( const FunctionCallbackInfo<Value>& args ) {
	Isolate* isolate = Isolate::GetCurrent();

	VALIDATE_ARGUMENT_COUNT( isolate, args, 3 );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 0, IsNumber );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 1, IsNumber );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 2, IsObject );

	args.GetReturnValue().Set(
		Buffer::Use( isolate,
			(char *)CreateNewOptionNode(
				( unsigned short )args[ 0 ]->Uint32Value(),
				( unsigned int )args[ 1 ]->Uint32Value(),
				(unsigned char *)Buffer::Data( args[ 2 ]->ToObject() ) ),
			sizeof( coap_list_t ) ) );

}

void bind_FormOCClientResponse( const FunctionCallbackInfo<Value>& args ) {
	Isolate *isolate = Isolate::GetCurrent();

	VALIDATE_ARGUMENT_COUNT( isolate, args, 5 );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 0, IsObject );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 1, IsUint32 );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 2, IsObject );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 3, IsUint32 );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 4, IsString );

	args.GetReturnValue().Set(
		Number::New( isolate,
			( double )FormOCClientResponse(
				( OCClientResponse* )Buffer::Data( args[ 0 ]->ToObject() ),
				( OCStackResult )args[ 1 ]->Uint32Value(),
				( OCDevAddr* )Buffer::Data( args[ 2 ]->ToObject() ),
				( uint32_t )args[ 3 ]->Uint32Value(),
				( const unsigned char * )*String::Utf8Value( args[ 4 ] ) ) ) );
}

void InitFunctions( Handle<Object> exports ) {
	NODE_SET_METHOD( exports, "hello", bind_hello );
	NODE_SET_METHOD( exports, "CoAPToOCResponseCode", bind_CoAPToOCResponseCode );
	NODE_SET_METHOD( exports, "CreateNewOptionNode", bind_CreateNewOptionNode );
	NODE_SET_METHOD( exports, "FormOCClientResponse", bind_FormOCClientResponse );
}
