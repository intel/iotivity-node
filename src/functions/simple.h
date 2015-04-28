#ifndef __IOTIVITY_NODE_FUNCTIONS_SIMPLE_H__
#define __IOTIVITY_NODE_FUNCTIONS_SIMPLE_H__

#include <v8.h>

void bind_OCInit( const v8::FunctionCallbackInfo<v8::Value>& args );
void bind_OCStop( const v8::FunctionCallbackInfo<v8::Value>& args );
void bind_OCStartPresence( const v8::FunctionCallbackInfo<v8::Value>& args );
void bind_OCStopPresence( const v8::FunctionCallbackInfo<v8::Value>& args );

#endif /* __IOTIVITY_NODE_FUNCTIONS_SIMPLE_H__ */
