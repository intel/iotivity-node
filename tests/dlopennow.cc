extern "C" {
#include <dlfcn.h>
}
#include <nan.h>
#include <node.h>
#include <v8.h>

using namespace v8;

NAN_METHOD( bind_dlopennow ) {
	void *theLibrary = 0;
	dlerror();

	if ( !info[ 0 ]->IsString() ) {
		Nan::ThrowTypeError( "Argument must be a string" );
		return;
	}

	theLibrary = dlopen( (const char *)*String::Utf8Value(info[0]), RTLD_NOW );

	char *theError = dlerror();

	if ( theError ) {
		if ( theLibrary ) {
			dlclose( theLibrary );
		}
		info.GetReturnValue().Set( Nan::New( theError ).ToLocalChecked() );
		return;
	}

	if ( theLibrary ) {
		dlclose( theLibrary );
	}
}

NAN_MODULE_INIT(Init) {
	Nan::Set( target, Nan::New( "dlopennow" ).ToLocalChecked(),
		Nan::GetFunction( Nan::New<FunctionTemplate>( bind_dlopennow ) ).ToLocalChecked() );
}

NODE_MODULE( dlopennow, Init )
