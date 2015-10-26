#ifndef __IOTIVITY_NODE_ENUMS_H__
#define __IOTIVITY_NODE_ENUMS_H__

#include <v8.h>

void InitEnums(v8::Handle<v8::Object> exports);

#define SET_CONSTANT_NUMBER(destination, name) \
  Nan::Set((destination), Nan::New(#name).ToLocalChecked(), Nan::New((name)));
#define SET_CONSTANT_STRING(destination, name)              \
  Nan::Set((destination), Nan::New(#name).ToLocalChecked(), \
           Nan::New((name)).ToLocalChecked());
#endif /* __IOTIVITY_NODE_ENUMS_H__ */
