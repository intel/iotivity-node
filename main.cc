#include <node.h>
#include <v8.h>

#include "functions.h"

using namespace v8;

void Init( Handle<Object> exports ) {
	Isolate* isolate = Isolate::GetCurrent();

	InitFunctions( exports, isolate );
}

NODE_MODULE( iotivity, Init )
