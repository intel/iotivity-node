#include <node.h>
#include <v8.h>

#include "functions.h"
#include "enums.h"

using namespace v8;

void Init( Handle<Object> exports ) {
	InitFunctions( exports );
	InitEnums( exports );
}

NODE_MODULE( iotivity, Init )
