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
#include "../structures/oc-entity-handler-response.h"
#include "../structures/oc-payload.h"

extern "C" {
#include <ocpayload.h>
#include <ocstack.h>
}

using namespace v8;

NAN_METHOD(bind_OCDoResponse) {
  VALIDATE_ARGUMENT_COUNT(info, 1);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);

  OCEntityHandlerResponse response;
  OCStackResult result;

  if (!c_OCEntityHandlerResponse(Nan::To<Object>(info[0]).ToLocalChecked(),
                                 &response)) {
    return;
  }

  result = OCDoResponse(&response);
  if (response.payload) {
    OCPayloadDestroy(response.payload);
  }
  info.GetReturnValue().Set(Nan::New(result));
}

NAN_METHOD(bind_OCNotifyAllObservers) {
  VALIDATE_ARGUMENT_COUNT(info, 2);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsUint32);

  CallbackInfo<OCResourceHandle> *callbackInfo;
  JSCALLBACKHANDLE_RESOLVE(JSOCResourceHandle, callbackInfo,
                           Nan::To<Object>(info[0]).ToLocalChecked());
  info.GetReturnValue().Set(Nan::New(OCNotifyAllObservers(

      callbackInfo->handle,
      (OCQualityOfService)Nan::To<uint32_t>(info[1]).FromJust())));
}

NAN_METHOD(bind_OCNotifyListOfObservers) {
  VALIDATE_ARGUMENT_COUNT(info, 4);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsObject);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsArray);
  VALIDATE_ARGUMENT_TYPE(info, 2, IsObject);
  VALIDATE_ARGUMENT_TYPE(info, 3, IsUint32);

  CallbackInfo<OCResourceHandle> *callbackInfo;
  JSCALLBACKHANDLE_RESOLVE(JSOCResourceHandle, callbackInfo,
                           Nan::To<Object>(info[0]).ToLocalChecked());

  // Construct the C array of observation IDs.
  Local<Array> obsIds = Local<Array>::Cast(info[1]);
  uint8_t arrayLength = (uint8_t)obsIds->Length();

  OCObservationId *c_observations = 0;
  int index;
  Local<Value> oneObservationId;
  OCRepPayload *payload = 0;
  Local<Number> returnValue;

  if (arrayLength > 0) {
    c_observations =
        (OCObservationId *)malloc(arrayLength * sizeof(OCObservationId));
    if (!c_observations && arrayLength > 0) {
      Nan::ThrowError(
          "OCNotifyListOfObservers: Failed to allocate list of observers");
      return;
    }
  }

  // Populate a C-like array from the V8 array
  for (index = 0; index < arrayLength; index++) {
    oneObservationId = Nan::Get(obsIds, index).ToLocalChecked();
    VALIDATE_VALUE_TYPE(oneObservationId, IsUint32, "obsId", goto free);
    c_observations[index] =
        (OCObservationId)Nan::To<uint32_t>(oneObservationId).FromJust();
  }

  if (!c_OCPayload(Nan::To<Object>(info[2]).ToLocalChecked(),
                   (OCPayload **)&payload)) {
    goto free;
  }

  returnValue = Nan::New(OCNotifyListOfObservers(
      callbackInfo->handle, c_observations, arrayLength, payload,
      (OCQualityOfService)Nan::To<uint32_t>(info[3]).FromJust()));

  OCRepPayloadDestroy(payload);
  info.GetReturnValue().Set(returnValue);
free:
  free(c_observations);
}
