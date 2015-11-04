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
  VALIDATE_ARGUMENT_COUNT(info, 3);
  VALIDATE_ARGUMENT_TYPE_OR_NULL(info, 0, IsString);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsUint32);
  VALIDATE_ARGUMENT_TYPE(info, 2, IsUint32);

  info.GetReturnValue().Set(Nan::New(OCInit(
      (const char *)(info[0]->IsString() ? (*String::Utf8Value(info[0])) : 0),
      (uint16_t)info[1]->Uint32Value(), (OCMode)info[2]->Uint32Value())));
}

NAN_METHOD(bind_OCStop) { info.GetReturnValue().Set(Nan::New(OCStop())); }

NAN_METHOD(bind_OCProcess) { info.GetReturnValue().Set(Nan::New(OCProcess())); }

NAN_METHOD(bind_OCStartPresence) {
  VALIDATE_ARGUMENT_COUNT(info, 1);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsUint32);

  info.GetReturnValue().Set(
      Nan::New(OCStartPresence((uint32_t)info[0]->Uint32Value())));
}

NAN_METHOD(bind_OCStopPresence) {
  info.GetReturnValue().Set(Nan::New(OCStopPresence()));
}

static bool c_OCDeviceInfo(Local<Object> devInfo, OCDeviceInfo *info) {
  Local<Value> deviceName =
      Nan::Get(devInfo, Nan::New("deviceName").ToLocalChecked())
          .ToLocalChecked();
  VALIDATE_VALUE_TYPE(deviceName, IsString, "deviceInfo.deviceName", false);
  char *devName;
  if (!c_StringNew(deviceName->ToString(), &devName)) {
    return false;
  }
  info->deviceName = devName;
  return true;
}

NAN_METHOD(bind_OCSetDeviceInfo) {
  VALIDATE_ARGUMENT_COUNT(info, 1);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);

  OCDeviceInfo deviceInfo;

  if (!c_OCDeviceInfo(info[0]->ToObject(), &deviceInfo)) {
    return;
  }

  OCStackResult result = OCSetDeviceInfo(deviceInfo);

  free(deviceInfo.deviceName);

  info.GetReturnValue().Set(Nan::New(result));
}

NAN_METHOD(bind_OCSetPlatformInfo) {
  VALIDATE_ARGUMENT_COUNT(info, 1);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);

  OCPlatformInfo platformInfo;

  if (!c_OCPlatformInfo(info[0]->ToObject(), &platformInfo)) {
    return;
  }

  OCStackResult result = OCSetPlatformInfo(platformInfo);

  c_OCPlatformInfoFreeMembers(&platformInfo);

  info.GetReturnValue().Set(Nan::New(result));
}

NAN_METHOD(bind_OCGetNumberOfResourceInterfaces) {
  VALIDATE_ARGUMENT_COUNT(info, 2);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsArray);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsObject);

  OCResourceHandle handle = 0;
  uint8_t interfaceCount = 0;
  OCStackResult result;

  if (!c_OCResourceHandle(Local<Array>::Cast(info[0]), &handle)) {
    return;
  }

  result = OCGetNumberOfResourceInterfaces(handle, &interfaceCount);

  Nan::Set(info[1]->ToObject(), Nan::New("count").ToLocalChecked(),
           Nan::New(interfaceCount));

  info.GetReturnValue().Set(Nan::New(result));
}

NAN_METHOD(bind_OCGetNumberOfResourceTypes) {
  VALIDATE_ARGUMENT_COUNT(info, 2);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsArray);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsObject);

  OCResourceHandle handle = 0;
  uint8_t typeCount = 0;
  OCStackResult result;

  if (!c_OCResourceHandle(Local<Array>::Cast(info[0]), &handle)) {
    return;
  }

  result = OCGetNumberOfResourceInterfaces(handle, &typeCount);

  Nan::Set(info[1]->ToObject(), Nan::New("count").ToLocalChecked(),
           Nan::New(typeCount));

  info.GetReturnValue().Set(Nan::New(result));
}

NAN_METHOD(bind_OCGetNumberOfResources) {
  VALIDATE_ARGUMENT_COUNT(info, 1);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);

  OCStackResult result;
  uint8_t resourceCount = 0;

  result = OCGetNumberOfResources(&resourceCount);

  Nan::Set(info[0]->ToObject(), Nan::New("count").ToLocalChecked(),
           Nan::New(resourceCount));

  info.GetReturnValue().Set(Nan::New(result));
}

NAN_METHOD(bind_OCGetResourceHandle) {
  VALIDATE_ARGUMENT_COUNT(info, 1);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsUint32);

  OCResourceHandle handle = 0;

  handle = OCGetResourceHandle((uint8_t)(info[0]->Uint32Value()));

  if (handle) {
    info.GetReturnValue().Set(js_OCResourceHandle(handle));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

#define RESOURCE_BY_INDEX_ACCESSOR_BOILERPLATE()                   \
  OCResourceHandle handle = 0;                                     \
  VALIDATE_ARGUMENT_COUNT(info, 2);                                \
  VALIDATE_ARGUMENT_TYPE(info, 0, IsArray);                        \
  VALIDATE_ARGUMENT_TYPE(info, 1, IsUint32);                       \
  if (!c_OCResourceHandle(Local<Array>::Cast(info[0]), &handle)) { \
    return;                                                        \
  }

NAN_METHOD(bind_OCGetResourceHandleFromCollection) {
  RESOURCE_BY_INDEX_ACCESSOR_BOILERPLATE();

  OCResourceHandle resourceHandle =
      OCGetResourceHandleFromCollection(handle, info[1]->Uint32Value());

  if (resourceHandle) {
    info.GetReturnValue().Set(js_OCResourceHandle(resourceHandle));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

#define GET_STRING_FROM_RESOURCE_BY_INDEX_BOILERPLATE(apiFunction) \
  RESOURCE_BY_INDEX_ACCESSOR_BOILERPLATE();                        \
  const char *resultOf##apiFunction =                              \
      apiFunction(handle, info[1]->Uint32Value());                 \
  if (resultOf##apiFunction) {                                     \
    info.GetReturnValue().Set(                                     \
        Nan::New(resultOf##apiFunction).ToLocalChecked());         \
  } else {                                                         \
    info.GetReturnValue().Set(Nan::Null());                        \
  }

NAN_METHOD(bind_OCGetResourceInterfaceName) {
  GET_STRING_FROM_RESOURCE_BY_INDEX_BOILERPLATE(OCGetResourceInterfaceName);
}

NAN_METHOD(bind_OCGetResourceTypeName) {
  GET_STRING_FROM_RESOURCE_BY_INDEX_BOILERPLATE(OCGetResourceTypeName);
}

#define LONE_ARGUMENT_IS_RESOURCE_HANDLE_BOILERPLATE()             \
  VALIDATE_ARGUMENT_COUNT(info, 1);                                \
  VALIDATE_ARGUMENT_TYPE(info, 0, IsArray);                        \
  OCResourceHandle handle = 0;                                     \
  if (!c_OCResourceHandle(Local<Array>::Cast(info[0]), &handle)) { \
    return;                                                        \
  }

NAN_METHOD(bind_OCGetResourceProperties) {
  LONE_ARGUMENT_IS_RESOURCE_HANDLE_BOILERPLATE();

  info.GetReturnValue().Set(Nan::New(OCGetResourceProperties(handle)));
}

NAN_METHOD(bind_OCGetResourceUri) {
  LONE_ARGUMENT_IS_RESOURCE_HANDLE_BOILERPLATE();

  const char *uri = OCGetResourceUri(handle);

  if (uri) {
    info.GetReturnValue().Set(Nan::New(uri).ToLocalChecked());
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

NAN_METHOD(bind_OCUnBindResource) {
  VALIDATE_ARGUMENT_COUNT(info, 2);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsArray);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsArray);

  OCResourceHandle collectionHandle = 0, resourceHandle = 0;

  if (!c_OCResourceHandle(Local<Array>::Cast(info[0]), &collectionHandle)) {
    return;
  }

  if (!c_OCResourceHandle(Local<Array>::Cast(info[1]), &resourceHandle)) {
    return;
  }

  info.GetReturnValue().Set(
      Nan::New(OCUnBindResource(collectionHandle, resourceHandle)));
}

#ifdef TESTING
NAN_METHOD(bind___compareResourceHandles) {
  VALIDATE_ARGUMENT_COUNT(info, 2);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsArray);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsArray);

  OCResourceHandle lhs = 0, rhs = 0;

  if (!c_OCResourceHandle(Local<Array>::Cast(info[0]), &lhs)) {
    return;
  }

  if (!c_OCResourceHandle(Local<Array>::Cast(info[1]), &rhs)) {
    return;
  }

  if (lhs == rhs) {
    info.GetReturnValue().Set(Nan::True());
  } else {
    info.GetReturnValue().Set(Nan::False());
  }
}

NAN_METHOD(bind___compareDoHandles) {
  VALIDATE_ARGUMENT_COUNT(info, 2);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsArray);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsArray);

  OCDoHandle lhs = 0, rhs = 0;

  if (!c_OCDoHandle(Local<Array>::Cast(info[0]), &lhs)) {
    return;
  }

  if (!c_OCDoHandle(Local<Array>::Cast(info[1]), &rhs)) {
    return;
  }

  if (lhs == rhs) {
    info.GetReturnValue().Set(Nan::True());
  } else {
    info.GetReturnValue().Set(Nan::False());
  }
}
#endif /* def TESTING */
