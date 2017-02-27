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

extern "C" {
#include <string.h>
}

#include "common.h"
#include "handles.h"
#include "oc-dev-addr.h"
#include "oc-entity-handler-request.h"
#include "oc-payload.h"

std::string js_OCEntityHandlerRequest(napi_env env,
                                      OCEntityHandlerRequest *request,
                                      napi_value *result) {
  NAPI_CALL_RETURN(napi_create_object(env, result));

  // The resource may be null if the request refers to a non-existing resource
  // and is being passed to the default device entity handler
  if (request->resource) {
    napi_ref jsRef = JSOCResourceHandle::handles[request->resource];
    if (!jsRef) {
      return LOCAL_MESSAGE("resource handle not found");
    }
    C2J_SET_PROPERTY_CALL_RETURN(
        env, *result, "resource",
        HELPER_CALL_RETURN(napi_get_reference_value(env, jsRef, &jsValue)));
  }

  if (request->requestHandle) {
    napi_value jsHandle;
    JSOCRequestHandle *cData;
    HELPER_CALL_RETURN(JSOCRequestHandle::New(env, &jsHandle, &cData));
    HELPER_CALL_RETURN(cData->Init(env, nullptr, jsHandle));
    C2J_SET_PROPERTY_JS_RETURN(env, *result, "requestHandle", jsHandle);
  } else {
    C2J_SET_PROPERTY_CALL_RETURN(
        env, *result, "requestHandle",
        NAPI_CALL_RETURN(napi_get_null(env, &jsValue)));
  }

  C2J_SET_NUMBER_MEMBER_RETURN(env, *result, request, method);
  C2J_SET_PROPERTY_CALL_RETURN(
      env, *result, "devAddr",
      HELPER_CALL_RETURN(js_OCDevAddr(env, &(request->devAddr), &jsValue)));
  C2J_SET_STRING_IF_NOT_NULL_RETURN(env, *result, request, query);

  napi_value jsObsInfo;
  NAPI_CALL_RETURN(napi_create_object(env, &jsObsInfo));
  C2J_SET_NUMBER_MEMBER_RETURN(env, jsObsInfo, &(request->obsInfo), action);
  C2J_SET_NUMBER_MEMBER_RETURN(env, jsObsInfo, &(request->obsInfo), obsId);
  C2J_SET_PROPERTY_JS_RETURN(env, *result, "obsInfo", jsObsInfo);

  C2J_SET_NUMBER_MEMBER_RETURN(env, *result, request, messageID);
  if (request->payload) {
    C2J_SET_PROPERTY_CALL_RETURN(
        env, *result, "payload",
        HELPER_CALL_RETURN(js_OCPayload(env, request->payload, &jsValue)));
  }

  // "rcvdVendorSpecificHeaderOptions" ignored

  return std::string();
}
/*
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
*/
