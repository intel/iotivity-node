#include <node.h>
#include <v8.h>

#include "functions/oc-create-delete-resource.h"
#include "functions/oc-do-resource.h"
#include "functions/simple.h"
#include "functions-internal.h"

using namespace v8;

v8::Persistent<v8::Object> *_callbacks;
int _uuidCounter;

void InitFunctions( Handle<Object> exports, Handle<Object> module ) {
	Isolate *isolate = Isolate::GetCurrent();

	// Initialize hash for storing JS callbacks
	_callbacks = new Persistent<Object>( isolate, Object::New( isolate ) );
	_uuidCounter = 0;

	NODE_SET_METHOD( exports, "OCInit", bind_OCInit );
	NODE_SET_METHOD( exports, "OCStop", bind_OCStop );
	NODE_SET_METHOD( exports, "OCCreateResource", bind_OCCreateResource );
	NODE_SET_METHOD( exports, "OCDeleteResource", bind_OCDeleteResource );
	NODE_SET_METHOD( exports, "OCStartPresence", bind_OCStartPresence );
	NODE_SET_METHOD( exports, "OCStopPresence", bind_OCStopPresence );
	NODE_SET_METHOD( exports, "OCDoResource", bind_OCDoResource );

#ifdef TESTING

	// For testing purposes we assign the callbacks object to the module exports so we can inspect
	// it from the JS test suite
	exports->Set( String::NewFromUtf8( isolate, "_test_callbacks" ),
		Local<Object>::New( isolate, callbacks ) );
#endif /* TESTING */
}
