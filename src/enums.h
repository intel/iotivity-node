#ifndef __IOTIVITY_NODE_ENUMS_H__
#define __IOTIVITY_NODE_ENUMS_H__

#include <node.h>
#include <v8.h>

using namespace v8;

void InitEnums(Handle<Object> exports);

#define SET_CONSTANT_MEMBER(destination, name)                      \
  (destination)                                                 \
      ->ForceSet(NanNew<String>(#name), NanNew<Number>((name)), \
                 static_cast<PropertyAttribute>(ReadOnly || DontDelete));


#endif /* __IOTIVITY_NODE_ENUMS_H__ */
