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

#include "../common.h"
#include "../structures/handles.h"
#include "../structures/oc-payload.h"

extern "C" {
#include <ocstack.h>
#include <ocpayload.h>
}

using namespace v8;
using namespace node;

NAN_METHOD(bind_OCNotifyAllObservers) {
  VALIDATE_ARGUMENT_COUNT(info, 2);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsArray);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsUint32);

  OCResourceHandle handle;
  if (!c_OCResourceHandle(Local<Array>::Cast(info[0]), &handle)) {
    return;
  }

  info.GetReturnValue().Set(Nan::New(OCNotifyAllObservers(
      handle, (OCQualityOfService)info[1]->Uint32Value())));
}

NAN_METHOD(bind_OCNotifyListOfObservers) {
  VALIDATE_ARGUMENT_COUNT(info, 4);
  VALIDATE_ARGUMENT_TYPE(info, 0, IsArray);
  VALIDATE_ARGUMENT_TYPE(info, 1, IsArray);
  VALIDATE_ARGUMENT_TYPE(info, 2, IsObject);
  VALIDATE_ARGUMENT_TYPE(info, 3, IsUint32);

  OCResourceHandle handle;
  if (!c_OCResourceHandle(Local<Array>::Cast(info[0]), &handle)) {
    return;
  }

  // Construct the C array of observation IDs.
  Local<Array> obsIds = Local<Array>::Cast(info[1]);
  uint8_t arrayLength = (uint8_t)obsIds->Length();

  OCObservationId *c_observations = 0;

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
  int index;
  for (index = 0; index < arrayLength; index++) {
    Local<Value> oneObservationId = Nan::Get(obsIds, index).ToLocalChecked();
    if (!(oneObservationId->IsUint32())) {
      free(c_observations);
      Nan::ThrowTypeError("OCObservationID must satisfy IsUint32()");
      return;
    }
    c_observations[index] = (OCObservationId)oneObservationId->Uint32Value();
  }

  OCRepPayload *payload = 0;
  if (!c_OCPayload(info[2]->ToObject(), (OCPayload **)&payload)) {
    free(c_observations);
    return;
  }

  Local<Number> returnValue = Nan::New(
      OCNotifyListOfObservers(handle, c_observations, arrayLength, payload,
                              (OCQualityOfService)info[3]->Uint32Value()));

  free(c_observations);
  OCRepPayloadDestroy(payload);

  info.GetReturnValue().Set(returnValue);
}
