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

#include "../common.h"
#include "../structures.h"

extern "C" {
#include <ocstack.h>
#include <stdlib.h>
}

using namespace v8;

Nan::Callback *g_currentCallback = 0;

static OCEntityHandlerResult defaultDeviceEntityHandler(
    OCEntityHandlerFlag flag, OCEntityHandlerRequest *request, char *uri,
    void *context) {
  Local<Value> jsCallbackArguments[3] = {
      Nan::New(flag), js_OCEntityHandlerRequest(request),
      (uri ? Nan::New<Value>((Handle<String>)Nan::New(uri).ToLocalChecked())
           : Nan::New<Value>((Handle<Primitive>)Nan::Undefined()))};

  Local<Value> returnValue =
      ((Nan::Callback *)context)->Call(3, jsCallbackArguments);

  VALIDATE_VALUE_TYPE(returnValue, IsUint32,
                      "OCDeviceEntityHandler return value", );

  return (OCEntityHandlerResult)(Nan::To<uint32_t>(returnValue).FromJust());
}

NAN_METHOD(bind_OCSetDefaultDeviceEntityHandler) {
  VALIDATE_ARGUMENT_COUNT(info, 1);
  VALIDATE_ARGUMENT_TYPE_OR_NULL(info, 0, IsFunction);

  OCDeviceEntityHandler newHandler = 0;
  Nan::Callback *newCallback = 0, *callbackToDelete = 0;

  if (info[0]->IsFunction()) {
    newCallback = new Nan::Callback(Local<Function>::Cast(info[0]));
    newHandler = defaultDeviceEntityHandler;
  }

  OCStackResult result =
      OCSetDefaultDeviceEntityHandler(newHandler, (void *)newCallback);

  if (result == OC_STACK_OK) {
    callbackToDelete = g_currentCallback;
    g_currentCallback = newCallback;
  } else {
    callbackToDelete = newCallback;
  }

  if (callbackToDelete) {
    delete callbackToDelete;
  }

  info.GetReturnValue().Set(Nan::New(result));
}
