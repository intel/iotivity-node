#ifndef __IOTIVITY_NODE_STRUCTURES_H__
#define __IOTIVITY_NODE_STRUCTURES_H__

#include <v8.h>

extern "C" {
#include <ocstack.h>
}

v8::Local<v8::Object> jsOCClientResponse( v8::Isolate *isolate, OCClientResponse *response );

#endif /* __IOTIVITY_NODE_STRUCTURES_H__ */
