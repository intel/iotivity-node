#ifndef __IOTIVITY_NODE_OC_CREATE_DELETE_RESOURCE_H__
#define __IOTIVITY_NODE_OC_CREATE_DELETE_RESOURCE_H__

#include <v8.h>

void bind_OCCreateResource( const v8::FunctionCallbackInfo<v8::Value>& args );
void bind_OCDeleteResource( const v8::FunctionCallbackInfo<v8::Value>& args );

#endif /* __IOTIVITY_NODE_OC_CREATE_DELETE_RESOURCE_H__ */
