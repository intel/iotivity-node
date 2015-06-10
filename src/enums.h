#ifndef __IOTIVITY_NODE_ENUMS_H__
#define __IOTIVITY_NODE_ENUMS_H__

#include <v8.h>

void InitEnums(v8::Handle<v8::Object> exports);

#define SET_CONSTANT_MEMBER(destination, v8Type, name)          \
  (destination)                                                 \
      ->ForceSet(NanNew<String>(#name), NanNew<v8Type>((name)), \
                 static_cast<PropertyAttribute>(ReadOnly || DontDelete));

#endif /* __IOTIVITY_NODE_ENUMS_H__ */
