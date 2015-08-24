#ifndef __IOTIVITY_NODE_STRING_PRIMITIVE_H__
#define __IOTIVITY_NODE_STRING_PRIMITIVE_H__

#include <v8.h>

bool c_StringNew(v8::Local<v8::String> jsString, char **p_string);

#endif /* __IOTIVITY_NODE_STRING_PRIMITIVE_H__ */
