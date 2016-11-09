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

#include "oc-entity-handler-response.h"
#include <nan.h>
#include "../common.h"
#include "handles.h"
#include "oc-dev-addr.h"
#include "oc-header-option-array.h"
#include "oc-payload.h"

extern "C" {
#include <ocpayload.h>
#include <string.h>
}

using namespace v8;

bool c_OCEntityHandlerResponse(Local<Object> jsResponse,
                               OCEntityHandlerResponse *p_response) {
  OCEntityHandlerResponse response = {
      0, 0, OC_EH_ERROR, 0, 0, {{OC_INVALID_ID, 0, 0, {0}}}, {0}, false};

  // requestHandle
  Local<Value> requestHandle =
      Nan::Get(jsResponse, Nan::New("requestHandle").ToLocalChecked())
          .ToLocalChecked();
  VALIDATE_VALUE_TYPE(requestHandle, IsObject,
                      "entitiy handler response request handle", return false);
  response.requestHandle = JSOCRequestHandle::Resolve(
      Nan::To<Object>(requestHandle).ToLocalChecked());
  if (!response.requestHandle) {
    return false;
  }

  // resourceHandle
  response.resourceHandle = 0;
  Local<Value> resourceHandle =
      Nan::Get(jsResponse, Nan::New("resourceHandle").ToLocalChecked())
          .ToLocalChecked();
  if (!(resourceHandle->IsUndefined() || resourceHandle->IsNull())) {
    VALIDATE_VALUE_TYPE(resourceHandle, IsObject,
                        "entitiy handler response resource handle",
                        return false);
    CallbackInfo<OCResourceHandle> *callbackInfo;
    JSCALLBACKHANDLE_RESOLVE(JSOCResourceHandle, callbackInfo,
                             Nan::To<Object>(resourceHandle).ToLocalChecked(),
                             false);
    response.resourceHandle = callbackInfo->handle;
  }

  // ehResult
  VALIDATE_AND_ASSIGN(response, jsResponse, ehResult, OCEntityHandlerResult,
                      IsUint32, "(entity handler response)", uint32_t,
                      return false);

  // payload
  response.payload = 0;
  Local<Value> payload =
      Nan::Get(jsResponse, Nan::New("payload").ToLocalChecked())
          .ToLocalChecked();
  if (!payload->IsNull()) {
    VALIDATE_VALUE_TYPE(payload, IsObject, "entity handler response payload",
                        return false);
    if (!c_OCPayload(Nan::To<Object>(payload).ToLocalChecked(),
                     &(response.payload))) {
      return false;
    }
  }

  //
  // After this point we need to free the payload whenever we bail.
  //

  Local<Value> resourceUri;
  size_t length;

  // sendVendorSpecificHeaderOptions and numSendVendorSpecificHeaderOptions
  Local<Value> sendVendorSpecificHeaderOptions =
      Nan::Get(jsResponse,
               Nan::New("sendVendorSpecificHeaderOptions").ToLocalChecked())
          .ToLocalChecked();
  VALIDATE_VALUE_TYPE(sendVendorSpecificHeaderOptions, IsArray,
                      "entity handler response header options", goto free);
  if (!c_OCHeaderOption(Local<Array>::Cast(sendVendorSpecificHeaderOptions),
                        response.sendVendorSpecificHeaderOptions,
                        &(response.numSendVendorSpecificHeaderOptions))) {
    goto free;
  }

  // resourceUri
  resourceUri = Nan::Get(jsResponse, Nan::New("resourceUri").ToLocalChecked())
                    .ToLocalChecked();
  VALIDATE_VALUE_TYPE(resourceUri, IsString,
                      "entity handler response resource URI", goto free);
  length = strlen((const char *)*String::Utf8Value(resourceUri));
  if (length >= MAX_URI_LENGTH) {
    Nan::ThrowRangeError(
        "entity handler response resource URI cannot fit inside MAX_URI_LENGTH "
        "(remember terminating zero)");
    goto free;
  }
  strcpy(response.resourceUri, (const char *)*String::Utf8Value(resourceUri));

  *p_response = response;
  return true;
free:
  OCPayloadDestroy(response.payload);
  return false;
}
