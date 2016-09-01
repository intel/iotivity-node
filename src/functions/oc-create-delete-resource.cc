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
#include <map>

#include "../common.h"
#include "../structures.h"
#include "../structures/handles.h"

extern "C" {
#include <ocstack.h>
}

using namespace v8;

static OCEntityHandlerResult defaultEntityHandler(
    OCEntityHandlerFlag flag, OCEntityHandlerRequest *request, void *context) {
  // Construct arguments to the JS callback and then call it, recording its
  // return value
  Local<Value> jsCallbackArguments[2] = {Nan::New(flag),
                                         js_OCEntityHandlerRequest(request)};
  Local<Value> returnValue;
  TRY_CALL(&(((CallbackInfo<OCResourceHandle> *)context)->callback),
           Nan::GetCurrentContext()->Global(), 2, jsCallbackArguments,
           returnValue, OC_EH_ERROR);

  VALIDATE_VALUE_TYPE(returnValue, IsUint32, "OCEntityHandler return value", );

  return (OCEntityHandlerResult)(Nan::To<uint32_t>(returnValue).FromJust());
}

NAN_METHOD(bind_OCCreateResource) {
  VALIDATE_ARGUMENT_COUNT(info, 6);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsString);
  VALIDATE_ARGUMENT_TYPE(info, 2, IsString);
  VALIDATE_ARGUMENT_TYPE(info, 3, IsString);
  VALIDATE_ARGUMENT_TYPE(info, 4, IsFunction);
  VALIDATE_ARGUMENT_TYPE(info, 5, IsUint32);

  CallbackInfo<OCResourceHandle> *callbackInfo =
      new CallbackInfo<OCResourceHandle>;
  if (!callbackInfo) {
    Nan::ThrowError("Failed to allocate memory for callback info");
    return;
  }

  OCStackResult returnValue = OCCreateResource(
      &(callbackInfo->handle), (const char *)*String::Utf8Value(info[1]),
      (const char *)*String::Utf8Value(info[2]),
      (const char *)*String::Utf8Value(info[3]), defaultEntityHandler,
      (void *)callbackInfo, (uint8_t)Nan::To<uint32_t>(info[5]).FromJust());

  if (returnValue == OC_STACK_OK) {
    Nan::Set(Nan::To<Object>(info[0]).ToLocalChecked(),
             Nan::New("handle").ToLocalChecked(),
             callbackInfo->Init(JSOCResourceHandle::New(callbackInfo),
                                Local<Function>::Cast(info[4])));
    JSOCResourceHandle::handles[callbackInfo->handle] =
        &(callbackInfo->jsHandle);
  } else {
    delete callbackInfo;
  }

  info.GetReturnValue().Set(Nan::New(returnValue));
}

NAN_METHOD(bind_OCDeleteResource) {
  VALIDATE_ARGUMENT_COUNT(info, 1);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);

  OCStackResult returnValue;

  CallbackInfo<OCResourceHandle> *callbackInfo;
  JSCALLBACKHANDLE_RESOLVE(JSOCResourceHandle, callbackInfo,
                           Nan::To<Object>(info[0]).ToLocalChecked());

  // Delete the resource identified by the handle
  returnValue = OCDeleteResource(callbackInfo->handle);

  if (returnValue == OC_STACK_OK) {
    JSOCResourceHandle::handles.erase(callbackInfo->handle);
    delete callbackInfo;
  }

  info.GetReturnValue().Set(Nan::New(returnValue));
}

// This is not really a binding since it only replaces the JS entity handler
NAN_METHOD(bind_OCBindResourceHandler) {
  VALIDATE_ARGUMENT_COUNT(info, 2);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsFunction);

  CallbackInfo<OCResourceHandle> *callbackInfo;
  JSCALLBACKHANDLE_RESOLVE(JSOCResourceHandle, callbackInfo,
                           Nan::To<Object>(info[0]).ToLocalChecked());
  callbackInfo->callback.Reset(Local<Function>::Cast(info[1]));

  info.GetReturnValue().Set(Nan::New(OC_STACK_OK));
}

#define BIND_UNBIND_RESOURCE(api)                                        \
  do {                                                                   \
    VALIDATE_ARGUMENT_COUNT(info, 2);                                    \
    VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);                           \
    VALIDATE_ARGUMENT_TYPE(info, 1, IsObject);                           \
    CallbackInfo<OCResourceHandle> *parentInfo;                          \
    JSCALLBACKHANDLE_RESOLVE(JSOCResourceHandle, parentInfo,             \
                             Nan::To<Object>(info[0]).ToLocalChecked()); \
    CallbackInfo<OCResourceHandle> *childInfo;                           \
    JSCALLBACKHANDLE_RESOLVE(JSOCResourceHandle, childInfo,              \
                             Nan::To<Object>(info[1]).ToLocalChecked()); \
    info.GetReturnValue().Set(                                           \
        Nan::New(api(parentInfo->handle, childInfo->handle)));           \
  } while (0)

NAN_METHOD(bind_OCBindResource) { BIND_UNBIND_RESOURCE(OCBindResource); }

NAN_METHOD(bind_OCUnBindResource) { BIND_UNBIND_RESOURCE(OCUnBindResource); }

#define BIND_STRING_TO_RESOURCE(api)                                        \
  do {                                                                      \
    VALIDATE_ARGUMENT_COUNT(info, 2);                                       \
    VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);                              \
    VALIDATE_ARGUMENT_TYPE(info, 1, IsString);                              \
    CallbackInfo<OCResourceHandle> *callbackInfo;                           \
    JSCALLBACKHANDLE_RESOLVE(JSOCResourceHandle, callbackInfo,              \
                             Nan::To<Object>(info[0]).ToLocalChecked());    \
    info.GetReturnValue().Set(Nan::New(api(                                 \
        callbackInfo->handle, (const char *)*String::Utf8Value(info[1])))); \
  } while (0)

NAN_METHOD(bind_OCBindResourceInterfaceToResource) {
  BIND_STRING_TO_RESOURCE(OCBindResourceInterfaceToResource);
}

NAN_METHOD(bind_OCBindResourceTypeToResource) {
  BIND_STRING_TO_RESOURCE(OCBindResourceTypeToResource);
}

// This is not actually a binding. We get the resource handler from the
// JS handle.
NAN_METHOD(bind_OCGetResourceHandler) {
  VALIDATE_ARGUMENT_COUNT(info, 1);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);

  CallbackInfo<OCResourceHandle> *callbackInfo;
  JSCALLBACKHANDLE_RESOLVE(JSOCResourceHandle, callbackInfo,
                           Nan::To<Object>(info[0]).ToLocalChecked());

  info.GetReturnValue().Set(*(callbackInfo->callback));
}
