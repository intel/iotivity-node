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

extern "C" {
#include <string.h>
}

#include "common.h"
#include "structures.h"
#include "structures/handles.h"
#include "structures/oc-dev-addr.h"
#include "structures/oc-header-option-array.h"
#include "structures/oc-payload.h"

using namespace v8;

Local<Object> js_OCEntityHandlerRequest(OCEntityHandlerRequest *request) {
  Local<Object> jsRequest = Nan::New<Object>();

  // The resource may be null if the request refers to a non-existing resource
  // and is being passed to the default device entity handler
  if (request->resource) {
    if (JSOCResourceHandle::handles[request->resource]->IsEmpty()) {
      Nan::ThrowError(
          "Conveying OCEntityHandlerRequest to JS: "
          "Failed to find JS resource handle");
    } else {
      Nan::Set(jsRequest, Nan::New("resource").ToLocalChecked(),
               Nan::New(*(JSOCResourceHandle::handles[request->resource])));
    }
  }

  if (request->requestHandle) {
    Nan::Set(jsRequest, Nan::New("requestHandle").ToLocalChecked(),
             JSOCRequestHandle::New(request->requestHandle));
  } else {
    Nan::Set(jsRequest, Nan::New("requestHandle").ToLocalChecked(),
             Nan::Null());
  }

  SET_VALUE_ON_OBJECT(jsRequest, request, method, Number);
  SET_STRING_IF_NOT_NULL(jsRequest, request, query);

  Local<Object> obsInfo = Nan::New<Object>();
  SET_VALUE_ON_OBJECT(obsInfo, &(request->obsInfo), action, Number);
  SET_VALUE_ON_OBJECT(obsInfo, &(request->obsInfo), obsId, Number);
  Nan::Set(jsRequest, Nan::New("obsInfo").ToLocalChecked(), obsInfo);

  Nan::Set(jsRequest,
           Nan::New("rcvdVendorSpecificHeaderOptions").ToLocalChecked(),
           js_OCHeaderOption(request->rcvdVendorSpecificHeaderOptions,
                             request->numRcvdVendorSpecificHeaderOptions));

  Nan::Set(jsRequest, Nan::New("devAddr").ToLocalChecked(),
           js_OCDevAddr(&(request->devAddr)));
  if (request->payload) {
    Nan::Set(jsRequest, Nan::New("payload").ToLocalChecked(),
             js_OCPayload(request->payload));
  }

  return jsRequest;
}
