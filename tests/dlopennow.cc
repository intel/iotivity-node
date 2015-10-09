extern "C" {
#include <dlfcn.h>
}
#include <nan.h>
#include <node.h>
#include <v8.h>

using namespace v8;

NAN_METHOD( bind_dlopennow ) {
	NanScope();
	void *theLibrary = 0;
	dlerror();

	if ( !args[ 0 ]->IsString() ) {
		NanThrowTypeError( "Argument must be a string" );
		NanReturnUndefined();
	}

	theLibrary = dlopen( (const char *)*String::Utf8Value(args[0]), RTLD_NOW );

	char *theError = dlerror();

	if ( theError ) {
		if ( theLibrary ) {
			dlclose( theLibrary );
		}
		NanReturnValue( NanNew<String>( theError ) );
	}

	if ( theLibrary ) {
		dlclose( theLibrary );
	}
	NanReturnUndefined();
}

void Init(Handle<Object> exports, Handle<Object> module) {
	exports->Set( NanNew<String>( "dlopennow" ),
		NanNew<FunctionTemplate>( bind_dlopennow )->GetFunction() );
}

NODE_MODULE( dlopennow, Init )
