#ifndef __IOTIVITY_NODE_STRUCTURES_H__
#define __IOTIVITY_NODE_STRUCTURES_H__

#include <v8.h>

extern "C" {
#include <ocstack.h>
}

v8::Local<v8::Object> js_OCEntityHandlerRequest(
    OCEntityHandlerRequest *request);

#endif /* __IOTIVITY_NODE_STRUCTURES_H__ */
