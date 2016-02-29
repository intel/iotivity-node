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

#include <v8.h>
#include <nan.h>
#include <map>

#include "../common.h"
#include "../structures/handles.h"
#include "../structures.h"

extern "C" {
#include <ocstack.h>
}

using namespace v8;
using namespace node;

// Associate the callback info with a resource handle
static std::map<OCResourceHandle, Nan::Callback *> annotation;

static OCEntityHandlerResult defaultEntityHandler(
    OCEntityHandlerFlag flag, OCEntityHandlerRequest *request, void *context) {
  // Construct arguments to the JS callback and then call it, recording its
  // return value
  Local<Value> jsCallbackArguments[2] = {Nan::New(flag),
                                         js_OCEntityHandlerRequest(request)};
  Local<Value> returnValue =
      ((Nan::Callback *)context)->Call(2, jsCallbackArguments);

  VALIDATE_CALLBACK_RETURN_VALUE_TYPE(returnValue, IsUint32, "OCEntityHandler");

  return (OCEntityHandlerResult)(returnValue->Uint32Value());
}

NAN_METHOD(bind_OCCreateResource) {
  VALIDATE_ARGUMENT_COUNT(info, 6);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsString);
  VALIDATE_ARGUMENT_TYPE(info, 2, IsString);
  VALIDATE_ARGUMENT_TYPE(info, 3, IsString);
  VALIDATE_ARGUMENT_TYPE(info, 4, IsFunction);
  VALIDATE_ARGUMENT_TYPE(info, 5, IsUint32);

  OCResourceHandle handle = 0;
  Nan::Callback *callback = new Nan::Callback(Local<Function>::Cast(info[4]));

  Local<Number> returnValue = Nan::New(OCCreateResource(
      &handle, (const char *)*String::Utf8Value(info[1]),
      (const char *)*String::Utf8Value(info[2]),
      (const char *)*String::Utf8Value(info[3]), defaultEntityHandler,
      (void *)callback, (uint8_t)info[5]->Uint32Value()));

  // Save info to the handle
  annotation[handle] = callback;
  info[0]->ToObject()->Set(Nan::New("handle").ToLocalChecked(),
                           js_OCResourceHandle(handle));
  info.GetReturnValue().Set(returnValue);
}

NAN_METHOD(bind_OCDeleteResource) {
  VALIDATE_ARGUMENT_COUNT(info, 1);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsArray);

  OCStackResult returnValue;
  OCResourceHandle handle = 0;

  // Retrieve OCResourceHandle from JS object
  if (!c_OCResourceHandle(Local<Array>::Cast(info[0]), &handle)) {
    return;
  }

  // Delete the resource identified by the handle
  returnValue = OCDeleteResource(handle);

  if (returnValue == OC_STACK_OK) {
    // If deleting the resource worked, get rid of the entity handler
    Nan::Callback *callback = annotation[handle];
    annotation.erase(handle);
    if (callback) {
      delete callback;
    }
  }

  info.GetReturnValue().Set(Nan::New(returnValue));
}

NAN_METHOD(bind_OCBindResourceHandler) {
  VALIDATE_ARGUMENT_COUNT(info, 2);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsArray);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsFunction);

  OCResourceHandle handle = 0;

  // Retrieve OCResourceHandle from JS object
  if (!c_OCResourceHandle(Local<Array>::Cast(info[0]), &handle)) {
    return;
  }

  Nan::Callback *callback = new Nan::Callback(Local<Function>::Cast(info[1]));

  // Replace the existing entity handler with the new callback
  OCStackResult returnValue =
      OCBindResourceHandler(handle, defaultEntityHandler, (void *)callback);

  if (returnValue == OC_STACK_OK) {
    // If setting the new entity handler worked, get rid of the original entity
    // handler and associate the new one with the handle.
    Nan::Callback *oldCallback = annotation[handle];
    if (oldCallback) {
      delete oldCallback;
    }
    annotation[handle] = callback;
  } else {
    // If the stack was not able to make use of the new entity handler, get rid
    // of the reference we created above.
    delete callback;
  }

  info.GetReturnValue().Set(Nan::New(returnValue));
}

NAN_METHOD(bind_OCBindResource) {
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
      Nan::New(OCBindResource(collectionHandle, resourceHandle)));
}

NAN_METHOD(bind_OCBindResourceInterfaceToResource) {
  VALIDATE_ARGUMENT_COUNT(info, 2);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsArray);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsString);

  OCResourceHandle handle = 0;

  if (!c_OCResourceHandle(Local<Array>::Cast(info[0]), &handle)) {
    return;
  }

  info.GetReturnValue().Set(Nan::New(OCBindResourceInterfaceToResource(
      handle, (const char *)*String::Utf8Value(info[1]))));
}

NAN_METHOD(bind_OCBindResourceTypeToResource) {
  VALIDATE_ARGUMENT_COUNT(info, 2);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsArray);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsString);

  OCResourceHandle handle = 0;

  if (!c_OCResourceHandle(Local<Array>::Cast(info[0]), &handle)) {
    return;
  }

  info.GetReturnValue().Set(Nan::New(OCBindResourceTypeToResource(
      handle, (const char *)*String::Utf8Value(info[1]))));
}

// This is not actually a binding. We get the resource handler from the
// annotation.
NAN_METHOD(bind_OCGetResourceHandler) {
  VALIDATE_ARGUMENT_COUNT(info, 1);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsArray);

  OCResourceHandle handle = 0;

  if (!c_OCResourceHandle(Local<Array>::Cast(info[0]), &handle)) {
    return;
  }

  info.GetReturnValue().Set((*(*annotation[handle])));
}
