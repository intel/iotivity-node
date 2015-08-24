#ifndef __IOTIVITY_NODE_HANDLES_H__
#define __IOTIVITY_NODE_HANDLES_H__

#include <v8.h>
extern "C" {
#include <ocstack.h>
}

v8::Local<v8::Array> js_OCResourceHandle(OCResourceHandle handle);
bool c_OCResourceHandle(v8::Local<v8::Array> handle,
                        OCResourceHandle *p_cHandle);

v8::Local<v8::Array> js_OCRequestHandle(OCRequestHandle handle);
bool c_OCRequestHandle(v8::Local<v8::Array> handle, OCRequestHandle *p_cHandle);

v8::Local<v8::Array> js_OCDoHandle(OCDoHandle handle);
bool c_OCRequestHandle(v8::Local<v8::Array> handle, OCDoHandle *p_cHandle);

#endif /* __IOTIVITY_NODE_HANDLES_H__ */
