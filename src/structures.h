#ifndef __IOTIVITY_NODE_STRUCTURES_H__
#define __IOTIVITY_NODE_STRUCTURES_H__

#include <v8.h>

extern "C" {
#include <ocstack.h>
}

v8::Local<v8::Object> js_OCClientResponse(OCClientResponse *response);

// Fills in @destination. @destination->payload is assumed to be allocated of
// size
// MAX_RESPONSE_LENGTH
bool c_OCEntityHandlerResponse(OCEntityHandlerResponse *destination,
                               v8::Local<v8::Object> jsOCEntityHandlerResponse);

// Returns the Local<Object> that was passed in
v8::Local<v8::Object> js_OCResourceHandle(v8::Local<v8::Object> jsHandle,
                                          OCResourceHandle handle);

// Returns the C OCResourceHandle from the JS handle. Returns NULL and throws an
// exception if @jsHandle does not have the right keys
bool c_OCResourceHandle(OCResourceHandle *destination,
                        v8::Local<v8::Object> jsHandle);

v8::Local<v8::Object> js_OCEntityHandlerRequest(
    OCEntityHandlerRequest *request);

v8::Local<v8::Object> js_OCDevAddr(OCDevAddr *address);

// Fills in @address
bool c_OCDevAddr(v8::Local<v8::Object> jsDevAddr, OCDevAddr *address);

#endif /* __IOTIVITY_NODE_STRUCTURES_H__ */
