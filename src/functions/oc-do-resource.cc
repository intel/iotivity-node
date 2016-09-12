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
#include "../structures/handles.h"
#include "../structures/oc-client-response.h"
#include "../structures/oc-dev-addr.h"
#include "../structures/oc-header-option-array.h"
#include "../structures/oc-payload.h"

extern "C" {
#include <ocpayload.h>
#include <ocstack.h>
#include <stdlib.h>
}

using namespace v8;

static void deleteNanCallback(CallbackInfo<OCDoHandle> *handle) {
  Nan::HandleScope scope;

  delete handle;
}

// Create an object containing the information from an OCClientResponse
// structure
static OCStackApplicationResult defaultOCClientResponseHandler(
    void *context, OCDoHandle handle, OCClientResponse *clientResponse) {
  Nan::HandleScope scope;
  CallbackInfo<OCDoHandle> *callbackInfo = (CallbackInfo<OCDoHandle> *)context;

  // Call the JS Callback
  Local<Value> jsCallbackArguments[2] = {Nan::New(callbackInfo->jsHandle),
                                         js_OCClientResponse(clientResponse)};

  Local<Value> returnValue =
      TRY_CALL(&(callbackInfo->callback), Nan::GetCurrentContext()->Global(), 2,
               jsCallbackArguments, OC_STACK_KEEP_TRANSACTION);

  // Validate value we got back from it
  VALIDATE_CALLBACK_RETURN_VALUE_TYPE(returnValue, IsUint32,
                                      "OCClientResponseHandler");

  // Pass on the value to the C API
  return (OCStackApplicationResult)returnValue->Uint32Value();
}

NAN_METHOD(bind_OCDoResource) {
  VALIDATE_ARGUMENT_COUNT(info, 8);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsUint32);
  VALIDATE_ARGUMENT_TYPE(info, 2, IsString);
  VALIDATE_ARGUMENT_TYPE_OR_NULL(info, 3, IsObject);
  VALIDATE_ARGUMENT_TYPE_OR_NULL(info, 4, IsObject);
  VALIDATE_ARGUMENT_TYPE(info, 5, IsUint32);
  VALIDATE_ARGUMENT_TYPE(info, 6, IsUint32);
  VALIDATE_ARGUMENT_TYPE(info, 7, IsFunction);
  VALIDATE_ARGUMENT_TYPE_OR_NULL(info, 8, IsArray);

  OCDevAddr *destination = 0, destinationToFillIn;
  OCPayload *payload = 0;
  OCHeaderOption *options = 0;
  uint8_t optionCount = 0;
  OCCallbackData data;

  if (info[8]->IsArray()) {
    Local<Array> optionArray = Local<Array>::Cast(info[8]);
    size_t length = optionArray->Length();

    if (length > 0) {
      options = (OCHeaderOption *)malloc(length * sizeof(OCHeaderOption));
      if (!options) {
        Nan::ThrowError(
            "Ran out of memory attempting to allocate header options");
        return;
      }
      if (!c_OCHeaderOption(optionArray, options, &optionCount)) {
        free(options);
        return;
      }
    }
  }

  // If a destination is given, we only use it if it can be converted to a
  // OCDevAddr structure
  if (info[3]->IsObject()) {
    if (c_OCDevAddr(info[3]->ToObject(), &destinationToFillIn)) {
      destination = &destinationToFillIn;
    } else {
      free(options);
      return;
    }
  }

  // If a payload is given, we only use it if it can be converted to a
  // OCPayload*
  if (info[4]->IsObject()) {
    if (!c_OCPayload(info[4]->ToObject(), &payload)) {
      free(options);
      return;
    }
  }

  CallbackInfo<OCDoHandle> *callbackInfo = new CallbackInfo<OCDoHandle>;
  if (!callbackInfo) {
    Nan::ThrowError("OCDoResource: Failed to allocate callback info");
    free(options);
    return;
  }

  data.context = (void *)callbackInfo;
  data.cb = defaultOCClientResponseHandler;
  data.cd = (OCClientContextDeleter)deleteNanCallback;

  OCStackResult returnValue =
      OCDoResource(&(callbackInfo->handle), (OCMethod)info[1]->Uint32Value(),
                   (const char *)*String::Utf8Value(info[2]), destination,
                   payload, (OCConnectivityType)info[5]->Uint32Value(),
                   (OCQualityOfService)info[6]->Uint32Value(), &data, options,
                   (uint8_t)info[9]->Uint32Value());

  free(options);

  // We need not free the payload because it seems iotivity takes ownership.
  // Similarly, if OCDoResource() fails, iotivity calls the callback that frees
  // the data on our behalf.

  if (returnValue == OC_STACK_OK) {
    Nan::Set(Nan::To<Object>(info[0]).ToLocalChecked(),
             Nan::New("handle").ToLocalChecked(),
             callbackInfo->Init(JSOCDoHandle::New(callbackInfo),
                                Local<Function>::Cast(info[7])));
  }

  info.GetReturnValue().Set(Nan::New(returnValue));
}

NAN_METHOD(bind_OCCancel) {
  VALIDATE_ARGUMENT_COUNT(info, 3);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsUint32);
  VALIDATE_ARGUMENT_TYPE_OR_NULL(info, 2, IsArray);

  OCHeaderOption headerOptions[MAX_HEADER_OPTIONS] = {
      {OC_INVALID_ID, 0, 0, {0}}};
  uint8_t numberOfOptions = 0;

  if (info[2]->IsArray()) {
    if (!c_OCHeaderOption(Local<Array>::Cast(info[2]), headerOptions,
                          &numberOfOptions)) {
      return;
    }
  }

  info.GetReturnValue().Set(Nan::New(OCCancel(
      JSCALLBACKHANDLE_RESOLVE(JSOCDoHandle, OCDoHandle,
                               Nan::To<Object>(info[0]).ToLocalChecked())
          ->handle,
      (OCQualityOfService)info[1]->Uint32Value(), headerOptions,
      numberOfOptions)));
}
