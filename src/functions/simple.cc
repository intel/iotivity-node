#include <nan.h>
#include "../common.h"
#include "../structures/handles.h"
#include "../structures/oc-platform-info.h"
#include "../structures/string-primitive.h"

extern "C" {
#include <ocstack.h>
}

using namespace v8;

NAN_METHOD(bind_OCInit) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 3);
  VALIDATE_ARGUMENT_TYPE_OR_NULL(args, 0, IsString);
  VALIDATE_ARGUMENT_TYPE(args, 1, IsUint32);
  VALIDATE_ARGUMENT_TYPE(args, 2, IsNumber);

  NanReturnValue(NanNew<Number>(OCInit(
      (const char *)(args[0]->IsString() ? (*String::Utf8Value(args[0])) : 0),
      (uint16_t)args[1]->ToUint32()->Value(),
      (OCMode)args[2]->ToNumber()->Value())));
}

NAN_METHOD(bind_OCStop) {
  NanScope();

  NanReturnValue(NanNew<Number>(OCStop()));
}

NAN_METHOD(bind_OCProcess) {
  NanScope();

  NanReturnValue(NanNew<Number>(OCProcess()));
}

NAN_METHOD(bind_OCStartPresence) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 1);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsUint32);

  NanReturnValue(
      NanNew<Number>(OCStartPresence((uint32_t)args[0]->Uint32Value())));
}

NAN_METHOD(bind_OCStopPresence) {
  NanScope();

  NanReturnValue(NanNew<Number>(OCStopPresence()));
}

static bool c_OCDeviceInfo(Local<Object> devInfo, OCDeviceInfo *info) {
  Local<Value> deviceName = devInfo->Get(NanNew<String>("deviceName"));
  VALIDATE_VALUE_TYPE(deviceName, IsString, "deviceInfo.deviceName", false);
  char *devName;
  if (!c_StringNew(deviceName->ToString(), &devName)) {
    return false;
  }
  info->deviceName = devName;
  return true;
}

NAN_METHOD(bind_OCSetDeviceInfo) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 1);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsObject);

  OCDeviceInfo info;

  if (!c_OCDeviceInfo(args[0]->ToObject(), &info)) {
    NanReturnUndefined();
  }

  OCStackResult result = OCSetDeviceInfo(info);

  free(info.deviceName);

  NanReturnValue(NanNew<Number>(result));
}

NAN_METHOD(bind_OCSetPlatformInfo) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 1);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsObject);

  OCPlatformInfo info;

  if (!c_OCPlatformInfo(args[0]->ToObject(), &info)) {
    NanReturnUndefined();
  }

  OCStackResult result = OCSetPlatformInfo(info);

  c_OCPlatformInfoFreeMembers(&info);

  NanReturnValue(NanNew<Number>(result));
}

NAN_METHOD(bind_OCGetNumberOfResourceInterfaces) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 2);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsArray);
  VALIDATE_ARGUMENT_TYPE(args, 1, IsObject);

  OCResourceHandle handle = 0;
  uint8_t interfaceCount = 0;
  OCStackResult result;

  if (!c_OCResourceHandle(Local<Array>::Cast(args[0]), &handle)) {
    NanReturnUndefined();
  }

  result = OCGetNumberOfResourceInterfaces(handle, &interfaceCount);

  args[1]->ToObject()->Set(NanNew<String>("count"),
                           NanNew<Number>(interfaceCount));

  NanReturnValue(NanNew<Number>(result));
}

NAN_METHOD(bind_OCGetNumberOfResourceTypes) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 2);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsArray);
  VALIDATE_ARGUMENT_TYPE(args, 1, IsObject);

  OCResourceHandle handle = 0;
  uint8_t typeCount = 0;
  OCStackResult result;

  if (!c_OCResourceHandle(Local<Array>::Cast(args[0]), &handle)) {
    NanReturnUndefined();
  }

  result = OCGetNumberOfResourceInterfaces(handle, &typeCount);

  args[1]->ToObject()->Set(NanNew<String>("count"), NanNew<Number>(typeCount));

  NanReturnValue(NanNew<Number>(result));
}

NAN_METHOD(bind_OCGetNumberOfResources) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 1);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsObject);

  OCStackResult result;
  uint8_t resourceCount = 0;

  result = OCGetNumberOfResources(&resourceCount);

  args[0]->ToObject()->Set(NanNew<String>("count"),
                           NanNew<Number>(resourceCount));

  NanReturnValue(NanNew<Number>(result));
}

NAN_METHOD(bind_OCGetResourceHandle) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 1);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsUint32);

  OCResourceHandle handle = 0;

  handle = OCGetResourceHandle((uint8_t)(args[0]->Uint32Value()));

  if (handle) {
    NanReturnValue(js_OCResourceHandle(handle));
  } else {
    NanReturnNull();
  }
}

#define RESOURCE_BY_INDEX_ACCESSOR_BOILERPLATE()                   \
  NanScope();                                                      \
  OCResourceHandle handle = 0;                                     \
  VALIDATE_ARGUMENT_COUNT(args, 2);                                \
  VALIDATE_ARGUMENT_TYPE(args, 0, IsArray);                        \
  VALIDATE_ARGUMENT_TYPE(args, 1, IsUint32);                       \
  if (!c_OCResourceHandle(Local<Array>::Cast(args[0]), &handle)) { \
    NanReturnUndefined();                                          \
  }

NAN_METHOD(bind_OCGetResourceHandleFromCollection) {
  RESOURCE_BY_INDEX_ACCESSOR_BOILERPLATE();

  OCResourceHandle resourceHandle =
      OCGetResourceHandleFromCollection(handle, args[1]->Uint32Value());

  if (resourceHandle) {
    NanReturnValue(js_OCResourceHandle(resourceHandle));
  } else {
    NanReturnNull();
  }
}

#define GET_STRING_FROM_RESOURCE_BY_INDEX_BOILERPLATE(apiFunction) \
  RESOURCE_BY_INDEX_ACCESSOR_BOILERPLATE();                        \
  const char *resultOf##apiFunction =                              \
      apiFunction(handle, args[1]->Uint32Value());                 \
  if (resultOf##apiFunction) {                                     \
    NanReturnValue(NanNew<String>(resultOf##apiFunction));         \
  } else {                                                         \
    NanReturnNull();                                               \
  }

NAN_METHOD(bind_OCGetResourceInterfaceName) {
  GET_STRING_FROM_RESOURCE_BY_INDEX_BOILERPLATE(OCGetResourceInterfaceName);
}

NAN_METHOD(bind_OCGetResourceTypeName) {
  GET_STRING_FROM_RESOURCE_BY_INDEX_BOILERPLATE(OCGetResourceTypeName);
}

#define LONE_ARGUMENT_IS_RESOURCE_HANDLE_BOILERPLATE()             \
  NanScope();                                                      \
  VALIDATE_ARGUMENT_COUNT(args, 1);                                \
  VALIDATE_ARGUMENT_TYPE(args, 0, IsArray);                        \
  OCResourceHandle handle = 0;                                     \
  if (!c_OCResourceHandle(Local<Array>::Cast(args[0]), &handle)) { \
    NanReturnUndefined();                                          \
  }

NAN_METHOD(bind_OCGetResourceProperties) {
  LONE_ARGUMENT_IS_RESOURCE_HANDLE_BOILERPLATE();

  NanReturnValue(NanNew<Number>(OCGetResourceProperties(handle)));
}

NAN_METHOD(bind_OCGetResourceUri) {
  LONE_ARGUMENT_IS_RESOURCE_HANDLE_BOILERPLATE();

  const char *uri = OCGetResourceUri(handle);

  if (uri) {
    NanReturnValue(NanNew<String>(uri));
  } else {
    NanReturnNull();
  }
}

NAN_METHOD(bind_OCUnBindResource) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 2);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsArray);
  VALIDATE_ARGUMENT_TYPE(args, 1, IsArray);

  OCResourceHandle collectionHandle = 0, resourceHandle = 0;

  if (!c_OCResourceHandle(Local<Array>::Cast(args[0]), &collectionHandle)) {
    NanReturnUndefined();
  }

  if (!c_OCResourceHandle(Local<Array>::Cast(args[1]), &resourceHandle)) {
    NanReturnUndefined();
  }

  NanReturnValue(
      NanNew<Number>(OCUnBindResource(collectionHandle, resourceHandle)));
}

#ifdef TESTING
NAN_METHOD(bind___compareResourceHandles) {
  NanScope();

  VALIDATE_ARGUMENT_COUNT(args, 2);
  VALIDATE_ARGUMENT_TYPE(args, 0, IsArray);
  VALIDATE_ARGUMENT_TYPE(args, 1, IsArray);

  OCResourceHandle lhs = 0, rhs = 0;

  if (!c_OCResourceHandle(Local<Array>::Cast(args[0]), &lhs)) {
    NanReturnUndefined();
  }

  if (!c_OCResourceHandle(Local<Array>::Cast(args[1]), &rhs)) {
    NanReturnUndefined();
  }

  if (lhs == rhs) {
    NanReturnValue(NanTrue());
  } else {
    NanReturnValue(NanFalse());
  }
}
#endif /* def TESTING */
