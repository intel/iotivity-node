#include <node.h>
#include <nan.h>

#include "functions/oc-create-delete-resource.h"
#include "functions/oc-do-resource.h"
#include "functions/oc-do-response.h"
#include "functions/oc-notify.h"
#include "functions/simple.h"

using namespace v8;

void InitFunctions( Handle<Object> exports, Handle<Object> module ) {
	exports->Set( NanNew<String>( "OCInit" ),
		NanNew<FunctionTemplate>( bind_OCInit )->GetFunction() );
	exports->Set( NanNew<String>( "OCStop" ),
		NanNew<FunctionTemplate>( bind_OCStop )->GetFunction() );
	exports->Set( NanNew<String>( "OCProcess" ),
		NanNew<FunctionTemplate>( bind_OCProcess )->GetFunction() );
	exports->Set( NanNew<String>( "OCCreateResource" ),
		NanNew<FunctionTemplate>( bind_OCCreateResource )->GetFunction() );
	exports->Set( NanNew<String>( "OCBindResourceHandler" ),
		NanNew<FunctionTemplate>( bind_OCBindResourceHandler )->GetFunction() );
	exports->Set( NanNew<String>( "OCDeleteResource" ),
		NanNew<FunctionTemplate>( bind_OCDeleteResource )->GetFunction() );
	exports->Set( NanNew<String>( "OCStartPresence" ),
		NanNew<FunctionTemplate>( bind_OCStartPresence )->GetFunction() );
	exports->Set( NanNew<String>( "OCStopPresence" ),
		NanNew<FunctionTemplate>( bind_OCStopPresence )->GetFunction() );
	exports->Set( NanNew<String>( "OCDoResource" ),
		NanNew<FunctionTemplate>( bind_OCDoResource )->GetFunction() );
	exports->Set( NanNew<String>( "OCDoResponse" ),
		NanNew<FunctionTemplate>( bind_OCDoResponse )->GetFunction() );
	exports->Set( NanNew<String>( "OCNotifyAllObservers" ),
		NanNew<FunctionTemplate>( bind_OCNotifyAllObservers )->GetFunction() );
	exports->Set( NanNew<String>( "OCNotifyListOfObservers" ),
		NanNew<FunctionTemplate>( bind_OCNotifyListOfObservers )->GetFunction() );
}
