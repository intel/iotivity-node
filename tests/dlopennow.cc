/*
 * Copyright 2016 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
