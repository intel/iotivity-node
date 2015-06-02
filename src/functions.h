#ifndef __IOTIVITY_NODE_FUNCTIONS_H__
#define __IOTIVITY_NODE_FUNCTIONS_H__

#include <node.h>
#include <v8.h>

using namespace v8;

void InitFunctions(Handle<Object> exports, Handle<Object> module);

#endif /* __IOTIVITY_NODE_FUNCTIONS_H__ */
