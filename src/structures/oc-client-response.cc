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
#include "oc-client-response.h"
#include "oc-dev-addr.h"
#include "../common.h"
#include "oc-header-option-array.h"
#include "oc-payload.h"
#include "oc-identity.h"

extern "C" {
#include <string.h>
}

using namespace v8;

Local<Object> js_OCClientResponse(OCClientResponse *response) {
  Local<Object> returnValue = Nan::New<Object>();

  // response.devAddr
  Nan::Set(returnValue, Nan::New("devAddr").ToLocalChecked(),
           js_OCDevAddr(&(response->devAddr)));

  // response.addr
  if (response->addr) {
    Nan::Set(returnValue, Nan::New("addr").ToLocalChecked(),
             js_OCDevAddr(response->addr));
  }

  // response.payload
  if (response->payload) {
    Nan::Set(returnValue, Nan::New("payload").ToLocalChecked(),
             js_OCPayload(response->payload));
  }

  SET_VALUE_ON_OBJECT(returnValue, Number, response, connType);

  Nan::Set(returnValue, Nan::New("identity").ToLocalChecked(),
           js_OCIdentity(&(response->identity)));

  SET_VALUE_ON_OBJECT(returnValue, Number, response, result);
  SET_VALUE_ON_OBJECT(returnValue, Number, response, sequenceNumber);

  // FIXME - iotivity has a bug whereby these fields are left uninitialized in
  // a presence response
  if (!(response->payload &&
        response->payload->type == PAYLOAD_TYPE_PRESENCE)) {
    SET_STRING_IF_NOT_NULL(returnValue, response, resourceUri);

    // response.rcvdVendorSpecificHeaderOptions
    Nan::Set(returnValue,
             Nan::New("rcvdVendorSpecificHeaderOptions").ToLocalChecked(),
             js_OCHeaderOption(response->rcvdVendorSpecificHeaderOptions,
                               response->numRcvdVendorSpecificHeaderOptions));
  }

  return returnValue;
}
