#ifndef __IOTIVITY_NODE_ENUMS_H__
#define __IOTIVITY_NODE_ENUMS_H__

#include <v8.h>

void InitEnums(v8::Handle<v8::Object> exports);

#define SET_CONSTANT_NUMBER(destination, name)                   \
  Nan::ForceSet((destination), Nan::New(#name).ToLocalChecked(), \
                Nan::New((name)),                                \
                (v8::PropertyAttribute)(v8::ReadOnly | v8::DontDelete));
#define SET_CONSTANT_STRING(destination, name)                   \
  Nan::ForceSet((destination), Nan::New(#name).ToLocalChecked(), \
                Nan::New((name)).ToLocalChecked(),               \
                (v8::PropertyAttribute)(v8::ReadOnly | v8::DontDelete));
#endif /* __IOTIVITY_NODE_ENUMS_H__ */
