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

// Returns the Local<Object> that was passed in
v8::Local<v8::Object> js_OCResourceHandle(
	v8::Local<v8::Object> jsHandle,
	OCResourceHandle handle );

// Returns the C OCResourceHandle from the JS handle. @p_info may be null if you don't care
// about the callback info. Return NULL and throws an exception if @jsHandle does not have the
// right keys
OCResourceHandle c_OCResourceHandle( v8::Local<v8::Object> jsHandle );

v8::Local<v8::Object> js_OCEntityHandlerRequest( OCEntityHandlerRequest *request );

#endif /* __IOTIVITY_NODE_STRUCTURES_H__ */
