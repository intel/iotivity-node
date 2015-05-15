#ifndef __IOTIVITY_NODE_STRUCTURES_H__
#define __IOTIVITY_NODE_STRUCTURES_H__

#include <v8.h>

extern "C" {
#include <ocstack.h>
}

v8::Local<v8::Object> js_OCClientResponse( OCClientResponse *response );

// Fills in @destination. @destination->payload is assumed to be allocated of size
// MAX_RESPONSE_LENGTH
bool c_OCEntityHandlerResponse(
	OCEntityHandlerResponse *destination,
	v8::Local<v8::Object> jsOCEntityHandlerResponse );

#endif /* __IOTIVITY_NODE_STRUCTURES_H__ */
