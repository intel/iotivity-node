#ifndef __IOTIVITY_NODE_OCHEADEROPTION_ARRAY__
#define __IOTIVITY_NODE_OCHEADEROPTION_ARRAY__

#include <v8.h>
extern "C" {
#include <ocstack.h>
}

v8::Local<v8::Array> js_OCHeaderOption( OCHeaderOption *options, uint8_t optionCount );
bool c_OCHeaderOption( v8::Local<v8::Array> jsOptions, OCHeaderOption *p_options, uint8_t *p_optionCount );

#endif /* __IOTIVITY_NODE_OCHEADEROPTION_ARRAY__ */
