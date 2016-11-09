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

extern "C" {
#include <ocstack.h>
}

using namespace v8;

#define GET_STRING_COUNT(api)                                                 \
  do {                                                                        \
    VALIDATE_ARGUMENT_COUNT(info, 2);                                         \
    VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);                                \
    VALIDATE_ARGUMENT_TYPE(info, 1, IsObject);                                \
                                                                              \
    uint8_t interfaceCount = 0;                                               \
    OCStackResult result;                                                     \
                                                                              \
    CallbackInfo<OCResourceHandle> *callbackInfo;                             \
    JSCALLBACKHANDLE_RESOLVE(JSOCResourceHandle, callbackInfo,                \
                             Nan::To<Object>(info[0]).ToLocalChecked());      \
    result = api(                                                             \
                                                                              \
        callbackInfo->handle, &interfaceCount);                               \
                                                                              \
    if (result == OC_STACK_OK) {                                              \
      Nan::Set(Nan::To<Object>(info[1]).ToLocalChecked(),                     \
               Nan::New("count").ToLocalChecked(), Nan::New(interfaceCount)); \
    }                                                                         \
                                                                              \
    info.GetReturnValue().Set(Nan::New(result));                              \
  } while (0)

NAN_METHOD(bind_OCGetNumberOfResourceInterfaces) {
  GET_STRING_COUNT(OCGetNumberOfResourceInterfaces);
}

NAN_METHOD(bind_OCGetNumberOfResourceTypes) {
  GET_STRING_COUNT(OCGetNumberOfResourceTypes);
}

NAN_METHOD(bind_OCGetResourceHandle) {
  VALIDATE_ARGUMENT_COUNT(info, 1);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsUint32);

  OCResourceHandle handle = 0;

  handle =
      OCGetResourceHandle((uint8_t)(Nan::To<uint32_t>(info[0]).FromJust()));

  if (handle) {
    if (JSOCResourceHandle::handles[handle]->IsEmpty()) {
      Nan::ThrowError("JS handle not found for native handle");
      return;
    }
    info.GetReturnValue().Set(Nan::New(*(JSOCResourceHandle::handles[handle])));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

#define RESOURCE_BY_INDEX_ACCESSOR_BOILERPLATE()             \
  VALIDATE_ARGUMENT_COUNT(info, 2);                          \
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);                 \
  VALIDATE_ARGUMENT_TYPE(info, 1, IsUint32);                 \
  CallbackInfo<OCResourceHandle> *callbackInfo;              \
  JSCALLBACKHANDLE_RESOLVE(JSOCResourceHandle, callbackInfo, \
                           Nan::To<Object>(info[0]).ToLocalChecked());

NAN_METHOD(bind_OCGetResourceHandleFromCollection) {
  RESOURCE_BY_INDEX_ACCESSOR_BOILERPLATE();

  OCResourceHandle resourceHandle = OCGetResourceHandleFromCollection(
      callbackInfo->handle, Nan::To<uint32_t>(info[1]).FromJust());

  if (resourceHandle) {
    if (JSOCResourceHandle::handles[resourceHandle]->IsEmpty()) {
      Nan::ThrowError("Failed to find JS resource handle");
      return;
    } else {
      info.GetReturnValue().Set(
          Nan::New(*(JSOCResourceHandle::handles[resourceHandle])));
    }
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

#define GET_STRING_FROM_RESOURCE_BY_INDEX_BOILERPLATE(apiFunction)  \
  RESOURCE_BY_INDEX_ACCESSOR_BOILERPLATE();                         \
  const char *resultOf##apiFunction = apiFunction(                  \
      callbackInfo->handle, Nan::To<uint32_t>(info[1]).FromJust()); \
  if (resultOf##apiFunction) {                                      \
    info.GetReturnValue().Set(                                      \
        Nan::New(resultOf##apiFunction).ToLocalChecked());          \
  } else {                                                          \
    info.GetReturnValue().Set(Nan::Null());                         \
  }

NAN_METHOD(bind_OCGetResourceInterfaceName) {
  GET_STRING_FROM_RESOURCE_BY_INDEX_BOILERPLATE(OCGetResourceInterfaceName);
}

NAN_METHOD(bind_OCGetResourceTypeName) {
  GET_STRING_FROM_RESOURCE_BY_INDEX_BOILERPLATE(OCGetResourceTypeName);
}

#define LONE_ARGUMENT_IS_RESOURCE_HANDLE_BOILERPLATE()       \
  VALIDATE_ARGUMENT_COUNT(info, 1);                          \
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);                 \
  CallbackInfo<OCResourceHandle> *callbackInfo;              \
  JSCALLBACKHANDLE_RESOLVE(JSOCResourceHandle, callbackInfo, \
                           Nan::To<Object>(info[0]).ToLocalChecked());

NAN_METHOD(bind_OCGetResourceProperties) {
  LONE_ARGUMENT_IS_RESOURCE_HANDLE_BOILERPLATE();

  info.GetReturnValue().Set(
      Nan::New(OCGetResourceProperties(callbackInfo->handle)));
}

NAN_METHOD(bind_OCGetResourceUri) {
  LONE_ARGUMENT_IS_RESOURCE_HANDLE_BOILERPLATE();

  const char *uri = OCGetResourceUri(callbackInfo->handle);

  if (uri) {
    info.GetReturnValue().Set(Nan::New(uri).ToLocalChecked());
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}
