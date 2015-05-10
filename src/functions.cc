#include <node.h>
#include <v8.h>

#include "functions/oc-create-delete-resource.h"
#include "functions/oc-do-resource.h"
#include "functions/simple.h"
#include "functions-internal.h"

using namespace v8;

void InitFunctions( Handle<Object> exports, Handle<Object> module ) {
	NODE_SET_METHOD( exports, "OCInit", bind_OCInit );
	NODE_SET_METHOD( exports, "OCStop", bind_OCStop );
	NODE_SET_METHOD( exports, "OCProcess", bind_OCProcess );
	NODE_SET_METHOD( exports, "OCCreateResource", bind_OCCreateResource );
	NODE_SET_METHOD( exports, "OCDeleteResource", bind_OCDeleteResource );
	NODE_SET_METHOD( exports, "OCStartPresence", bind_OCStartPresence );
	NODE_SET_METHOD( exports, "OCStopPresence", bind_OCStopPresence );
	NODE_SET_METHOD( exports, "OCDoResource", bind_OCDoResource );
}
