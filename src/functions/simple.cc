/*
 * Copyright 2016 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <nan.h>
#include "../common.h"
#include "../structures/handles.h"
#include "../structures/oc-platform-info.h"
#include "../structures/oc-device-info.h"
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

NAN_METHOD(bind_OCSetDeviceInfo) {
  VALIDATE_ARGUMENT_COUNT(info, 1);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);

  OCDeviceInfo deviceInfo;

  if (!c_OCDeviceInfo(info[0]->ToObject(), &deviceInfo)) {
    return;
  }

  OCStackResult result = OCSetDeviceInfo(deviceInfo);

  c_OCDeviceInfoFreeMembers(&deviceInfo);

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
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsObject);

  OCResourceHandle handle = 0;
  uint8_t interfaceCount = 0;
  OCStackResult result;

  if (!c_OCResourceHandle(Nan::To<Object>(info[0]).ToLocalChecked(), &handle)) {
    return;
  }

  result = OCGetNumberOfResourceInterfaces(handle, &interfaceCount);

  Nan::Set(info[1]->ToObject(), Nan::New("count").ToLocalChecked(),
           Nan::New(interfaceCount));

  info.GetReturnValue().Set(Nan::New(result));
}

NAN_METHOD(bind_OCGetNumberOfResourceTypes) {
  VALIDATE_ARGUMENT_COUNT(info, 2);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsObject);

  OCResourceHandle handle = 0;
  uint8_t typeCount = 0;
  OCStackResult result;

  if (!c_OCResourceHandle(Nan::To<Object>(info[0]).ToLocalChecked(), &handle)) {
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

#define RESOURCE_BY_INDEX_ACCESSOR_BOILERPLATE()                     \
  OCResourceHandle handle = 0;                                       \
  VALIDATE_ARGUMENT_COUNT(info, 2);                                  \
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);                         \
  VALIDATE_ARGUMENT_TYPE(info, 1, IsUint32);                         \
  if (!c_OCResourceHandle(Nan::To<Object>(info[0]).ToLocalChecked(), \
                          &handle)) {                                \
    return;                                                          \
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

#define LONE_ARGUMENT_IS_RESOURCE_HANDLE_BOILERPLATE()               \
  VALIDATE_ARGUMENT_COUNT(info, 1);                                  \
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);                         \
  OCResourceHandle handle = 0;                                       \
  if (!c_OCResourceHandle(Nan::To<Object>(info[0]).ToLocalChecked(), \
                          &handle)) {                                \
    return;                                                          \
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
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsObject);

  OCResourceHandle collectionHandle = 0, resourceHandle = 0;

  if (!c_OCResourceHandle(Nan::To<Object>(info[0]).ToLocalChecked(),
                          &collectionHandle)) {
    return;
  }

  if (!c_OCResourceHandle(Nan::To<Object>(info[1]).ToLocalChecked(),
                          &resourceHandle)) {
    return;
  }

  info.GetReturnValue().Set(
      Nan::New(OCUnBindResource(collectionHandle, resourceHandle)));
}

NAN_METHOD(bind_OCGetServerInstanceIDString) {
  VALIDATE_ARGUMENT_COUNT(info, 0);

  const char *idString = OCGetServerInstanceIDString();

  info.GetReturnValue().Set(idString ? (Nan::New(idString).ToLocalChecked())
                                     : Nan::EmptyString());
}
