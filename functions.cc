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

void bind_FormOCResponse( const FunctionCallbackInfo<Value>& args ) {
	Isolate *isolate = Isolate::GetCurrent();
	OCResponse *response = 0;
	OCStackResult returnValue;
	Handle<Array> returnArray;

	// We assume that the first argument of the C function, OCResponse **responseLoc, is really
	// a return value. Thus, this function will return an array of two elements where the first is
	// the return value of the C function, and the second is a Buffer containing the data received
	// via the first argument of the C function - if any.

	VALIDATE_ARGUMENT_COUNT( isolate, args, 3 );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 0, IsObject );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 1, IsUint32 );
	VALIDATE_ARGUMENT_TYPE( isolate, args, 2, IsObject );

	// Make native call
	returnValue = FormOCResponse(
		&response,
		( ClientCB* )Buffer::Data( args[ 0 ]->ToObject() ),
		( uint8_t )args[ 1 ]->Uint32Value(),
		( OCClientResponse* )Buffer::Data( args[ 2 ]->ToObject() ) );

	// Create the response array - length is 1 if response is NULL, otherwise we pass a reference
	// to the data stored in response in the second element
	returnArray = Array::New( isolate, response ? 2 : 1 );
	returnArray->Set( 0, Number::New( isolate, ( double )returnValue ) );
	if ( response ) {
		returnArray->Set( 1, Buffer::Use( isolate, (char *)response, sizeof( OCResponse ) ) );
	}

	args.GetReturnValue().Set( returnArray );
}

void InitFunctions( Handle<Object> exports ) {
	NODE_SET_METHOD( exports, "hello", bind_hello );
	NODE_SET_METHOD( exports, "CoAPToOCResponseCode", bind_CoAPToOCResponseCode );
	NODE_SET_METHOD( exports, "CreateNewOptionNode", bind_CreateNewOptionNode );
	NODE_SET_METHOD( exports, "FormOCClientResponse", bind_FormOCClientResponse );
	NODE_SET_METHOD( exports, "FormOCResponse", bind_FormOCResponse );
}
